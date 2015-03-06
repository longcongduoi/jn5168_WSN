/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include "Fio.h"

#include "Config.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* Точность используемая при преобразовании чисел через %f в Fio_printf() */

// Определения свойств чисел с плавающей точкой
#if defined SINGLE_PRECISION

    #define EXPONENT_MAX 	127// Максимальное значение порядка
    #define EXPONENT_MIN 	-126// Минимальное значение порядка
    #define EXPONENT_LENGTH 	7// Длина порядка (бит)
    #define MANTESSE_LENGTH 	23// Длина мантиссы (бит)

    #define FLOATING_POINT_TYPE float// Тип, используемый для представления чисел одинарной точности
    #define FLOATING_POINT_BIN 	uint32// Тип, используемый для проведения операций с битами числа

#elif defined DOUBLE_PRECISION

    #define EXPONENT_MAX 	1023// Максимальное значение, принимаемое порядком
    #define EXPONENT_MIN 	-1022// Минимальное значение, принимаемое порядком
    #define EXPONENT_LENGTH 	11// Длина порядка (бит)
    #define MANTESSE_LENGTH 	52// Длина мантиссы (бит)

    #define FLOATING_POINT_TYPE double// Тип, используемый для представления чисел двойной точности
    #define FLOATING_POINT_BIN 	uint64// Тип, используемый для проведения операций с битами числа

#endif

#define MULTIPLICATOR_VALUE  1000000000000000000LL// Множитель, позволяющий домножить число и избавиться от значений после точки
#define MULTIPLICATOR_ORDER  18// Порядок мультипликатора <2^64 = 18446744073709551616 -> 19 и значения которое может нести домноженное double*10^Multiplicator// Определяется типами (В данном случае ограничено возможностями double)// Ограничивает точность числа

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

// Состояния в которых может находится число с плавающей точкой
typedef enum{

    FLOATING_POINT_NORMAL = 0,// Число по-умолчанию нормальное
    FLOATING_POINT_SUBNORMAL,
    FLOATING_POINT_ZERO,
    FLOATING_POINT_INFINITY,
    FLOATING_POINT_NOT_A_NUMBER,

} FloatingPointState_e;

// Составляющие распакованного вещественного числа
typedef struct{

    FloatingPointState_e State;// Особое состояние в котором находится число

    int8 Sgn;// Знак числа с плавающей запятой
    int16 Expt;// Порядок числа

    FLOATING_POINT_BIN Mtsa;// Его мантисса

} FloatingPointUnits_s;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE void convertBcd2String(Bcd_t CurrentBcd, void (*putChar)(char c));// Преобразовать BCD в строку
PRIVATE void convertNum2String(FLOATING_POINT_BIN Number, void (*putChar)(char c), uint8 Base, bool isRealNumberConversion);// Преобразовать целое в строку
PRIVATE void convertFloatingPoint2String(FLOATING_POINT_TYPE FloatingPointNumber, void (*putChar)(char c));// Преобразовать вещественное в строку

PRIVATE FloatingPointUnits_s unpackFloatingPoint(FLOATING_POINT_TYPE FloatingPointNumber);// Распаковать число // Изъять знак, порядок мантиссу и определить состояние
PRIVATE FLOATING_POINT_TYPE packFloatingPoint(FloatingPointUnits_s FloatingPointUnits);// Запаковать число // Возможно использовать особое состояние в качестве входных данных, если не нормальное число

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

//----------------------------------------------------------------------------
//  Форматный вывод
//----------------------------------------------------------------------------
PUBLIC void Fio_printf(const char *fmt, va_list ap, void (*putChar)(char c)){

    int32 i = 0;
    FLOATING_POINT_TYPE f = 0;
    char* p;

    char *bp = (char *)fmt;
    char c;

    while ((c = *bp++)) {
        if (c != '%') {
            if (c == '\n'){
                putChar('\n');
                putChar('\r');
            } else {
                putChar(c);
            }
            continue;
        }

        switch ((c = *bp++)) {
        /* %d - show a decimal value */
        case 'd':
            convertNum2String(va_arg(ap, uint32), putChar, 10, FALSE);
            break;

        /* %x - show a value in hex */
        case 'x':
            putChar('0');
            putChar('x');
            convertNum2String(va_arg(ap, uint32), putChar, 16, FALSE);
            break;

        case 't':
            convertBcd2String(va_arg(ap, Bcd_t), putChar);
            break;


        /* %b - show a value in binary */
        case 'b':
            putChar('0');
            putChar('b');
            convertNum2String(va_arg(ap, uint32), putChar, 2, FALSE);
            break;

        /* %c - show a character */
        case 'c':
            putChar(va_arg(ap, int));
            break;

        case 'i':

            i = va_arg(ap, int32);
            if(i < 0){
                putChar('-');
                convertNum2String((~i)+1, putChar, 10, FALSE);
            } else {
                convertNum2String(i, putChar, 10, FALSE);
            }
            break;

        /* %f - show a float */
        case 'f':
            f = va_arg(ap, FLOATING_POINT_TYPE);
            convertFloatingPoint2String(f, putChar);
            break;

        /* %s - show a string */
        case 's':
            p = va_arg(ap, char *);
            do {
                putChar(*p++);
            } while (*p);
            break;

        /* %% - show a % character */
        case '%':
            putChar('%');
            break;

        /* %something else not handled ! */
        default:
            putChar('?');
            break;

        }
    }

    return;
}

//----------------------------------------------------------------------------
// Открытая функция преобразования числа в строку
//----------------------------------------------------------------------------
PUBLIC void Fio_convertNum2String(uint64 Number, void (*putChar)(char c), uint8 Base){

    convertNum2String((FLOATING_POINT_BIN)Number, putChar, Base, FALSE);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

PRIVATE void convertBcd2String(Bcd_t CurrentBcd, void (*putChar)(char c)){

    uint8 i = 0;

    //-- Здесь ставится задача убрать лишние нули при выводе числа и только тогда вывести число

    // Подсчитать количество нулей позади числа
    while(((CurrentBcd & (0xF<<(i*4))) == 0) && (i < BCD_FRACTION_LENGTH)){

	i++;
    };

    if(CurrentBcd < 0){

	putChar('-');
	CurrentBcd *= -1;
    }

    // Число без дробной части
    if((i-BCD_FRACTION_LENGTH) >= 0){

	// Избаится от дробной части числа
	CurrentBcd >>= 4*BCD_FRACTION_LENGTH;

	// Вывести число
	convertNum2String(CurrentBcd, putChar, 16, FALSE);
    }
    // Есть дробная часть
    else{

	union{

	    Bcd_t Cash;
	    uint8 Collector[BCD_WHOLE_LENGTH+BCD_FRACTION_LENGTH];
	}Bcd;// Объединение для ликвидации лишних цифр числа

	Bcd.Cash = CurrentBcd;

	//-- Вытащить целую часть числа
	uint8 k;
	for(k=0; k<((BCD_WHOLE_LENGTH+BCD_FRACTION_LENGTH)/2); k++){

	    if(k >= (BCD_WHOLE_LENGTH/2)) Bcd.Collector[k] = 0;
	}

	//-- Избаится от дробной части числа
	Bcd.Cash >>= 4*BCD_FRACTION_LENGTH;

	//-- Вывести целую часть
	convertNum2String(Bcd.Cash, putChar, 16, FALSE);

	putChar('.');

	Bcd.Cash = CurrentBcd;

	//-- Вытащить дробную чатсть
	for(k=0; k<((BCD_WHOLE_LENGTH+BCD_FRACTION_LENGTH)/2); k++){

	    if(k < (BCD_WHOLE_LENGTH/2)) Bcd.Collector[k] = 0;
	}

	Bcd.Cash >>= i*4;// Убрать лишние нули

	//-- Подсчитать количество нулей перед значащими цифрами в дробной части// Это когда, например, вот так 0.0021 - два нуля
	uint8 zeros = 0;
	for(k=0; k<(BCD_FRACTION_LENGTH - i); k++){

	    if((Bcd.Cash & 0xF<<(k*4)) == 0){

		zeros++;
	    }
	}

	//-- Вывести доплонительные нули
	for(k=0; k<zeros; k++){

	    putChar('0');
	}

	convertNum2String(Bcd.Cash , putChar, 16, FALSE);// Вывести дробь
    }
}

//----------------------------------------------------------------------------
// Локальная функция для преобразования в строку, также используется для преобразования вещественного числа
//----------------------------------------------------------------------------
PRIVATE void convertNum2String(FLOATING_POINT_BIN Number, void (*putChar)(char c), uint8 Base, bool isRealNumberConversion){

    // Длина буфера символов
    #define BUF_LENGTH 33

    // Буфер, куда помещается строка
    char buf[BUF_LENGTH];

    // Рабочий указатель на элементы строки
    char *p = buf + BUF_LENGTH;
    uint64 c, n;

    *--p = '\0';
    do {
        n = Number / (uint64)Base;
        c = Number - (n * Base);
        if (c < 10) {
            *--p = '0' + c;
        } else {
            *--p = 'a' + (c - 10);
        }
        Number /= (uint64)Base;
    } while (Number != 0);

    // Верхняя ограничивающий указатель при выходе за границы окна нулей
    char* UpperLimitingPointer = buf + BUF_LENGTH;

    // Найденное окно нулей
    uint8 FixedZerosWindow = 0;
    uint8 FixedNinesWindow = 0;

    // Если используется для преобразования дробной части вещественного числа, то учесть погрешность записи чисел (много лишних цифр между важной частью числа и остатками)
    if(isRealNumberConversion){

	// Восстановить число
	if(((buf + BUF_LENGTH - 1) - p) < MULTIPLICATOR_ORDER){

	    uint8 i;
	    char* pcash = p;

	    for(i=0; i < MULTIPLICATOR_ORDER - ((buf + BUF_LENGTH - 1) - pcash); i++){

		*--p = '0';
	    }
	}

	// Указатель, используемый для поиска
	char* SearchingPointer = p;

	// Найти окно между существенной и несущественной частью
	while(*SearchingPointer && (FixedZerosWindow <= MAX_FRACTION_DIGITS) &&  (FixedNinesWindow <= MAX_FRACTION_DIGITS)){

	    if(*SearchingPointer == '0'){

		// Если зафиксирован первый ноль сохранить длину
		if(FixedZerosWindow == 0) UpperLimitingPointer = SearchingPointer;

		// Инкрементировать значение ширины окна
		FixedZerosWindow++;
	    }
	    else{

		// Окно закончилось, но не превышено, обнулить счётчик ширины окна
		FixedZerosWindow = 0;
	    }

	    if(*SearchingPointer == '9'){

		// Если зафиксирована первая цифра, сохранить длину
		if(FixedNinesWindow == 0) UpperLimitingPointer = SearchingPointer;

		// Инкрементировать значение ширины окна
		FixedNinesWindow++;
	    }
	    else{

		// Окно закончилось, но не превышено, обнулить счётчик ширины окна
		FixedNinesWindow = 0;
	    }
	    SearchingPointer++;
	}

	// ЗДЕСЬ НУЖНО ДОБАВИТЬ ОКРУГЛЕНИЕ, ПРИ МАКСИМАЛЬНОЙ ДОПУСТИМОЙ ТОЧНОСТИ РАВНОЙ ГРАНИЦЕ ПОГРЕШНОСТИ


	// Установить новый конец строки, если превышена величина окна значимых чисел
	if (FixedZerosWindow > MAX_FRACTION_DIGITS){


	    *UpperLimitingPointer = '\0';
	}

	// Установить новый конец строки, если превышена величина окна и увеличить на еденицу предыдущюю цифру
	if (FixedNinesWindow > MAX_FRACTION_DIGITS){

	    // Если самый первый символ
	    if(UpperLimitingPointer == p){

		// Установить на его месте действительную цифру
		if (*UpperLimitingPointer == '9') *UpperLimitingPointer++ = '1';
		else *UpperLimitingPointer += 1;

		// Далее установить конец строки
		*UpperLimitingPointer = '\0';

	    // Если последующие символы
	    }else if(UpperLimitingPointer > p){

		// Установить конец строки
		*(UpperLimitingPointer--) = '\0';

		// Увеличить предыдущий сивол на еденицу
		*UpperLimitingPointer += 1;
	    }
	}
    }

    while (*p){
	putChar(*p);
	p++;
    }
}

//----------------------------------------------------------------------------
//  Преобразовать вещественное в строку
//----------------------------------------------------------------------------
PRIVATE void convertFloatingPoint2String(FLOATING_POINT_TYPE FloatingPointNumber, void (*putChar)(char c)){

	// Проверить предусловие о попадании числа в допустимый диапазо
	if(ABS(FloatingPointNumber) > MAX_INTEGER_VALUE){

	    // Если не выполняется выйти из функции
	    putChar('?');
	    return;
	}

	// Распаковать число
	FloatingPointUnits_s FloatingPointUnits = unpackFloatingPoint(FloatingPointNumber);

	// Вывести знак числа
	if(FloatingPointUnits.Sgn < 0) putChar('-');

	// Вывести значение в соответсвии с типом числа
	switch(FloatingPointUnits.State){

	    // Нормальное число
	    case FLOATING_POINT_NORMAL:

		/* Получить целую часть числа */

		// Если число имеет целую часть
		if(FloatingPointUnits.Expt >= 0){

		    // Сдвинуть мантиссу, оставив целую часть, использовать для конвертации функцию преобразования целых чисел
		    convertNum2String((FLOATING_POINT_BIN)(FloatingPointUnits.Mtsa>>(MANTESSE_LENGTH-FloatingPointUnits.Expt)), putChar, 10, FALSE);

		    // Убрать из мантиссы целую часть
		    FloatingPointUnits.Mtsa &= ~((0xFFFFFFFFFFFFFFFFLL)<<(FLOATING_POINT_BIN)(MANTESSE_LENGTH-(FloatingPointUnits.Expt)));

		    // Проверить, осталась ли в мантиссе дробная часть, тогда пересчитать порядок
		    if(FloatingPointUnits.Mtsa != 0){

			uint8 Idx = 0;
			uint64 Shift = 0xFFFFFFFFFFFFFFFFLL;
			uint64 MtsaCash = FloatingPointUnits.Mtsa;

			// Определить сколько бит в мантиссе занимает дробная часть
			while(MtsaCash & (Shift << Idx++));

			// Пересчитать порядок под дробную часть
			FloatingPointUnits.Expt = FloatingPointUnits.Expt - (54 - Idx);

			// Сместить дробную часть к началу мантиссы
			FloatingPointUnits.Mtsa <<= 54 - Idx;
		    }
		    // Если мантисса всё же равна нулю -> вывести ноль
		    else{

			putChar('.');
			putChar('0');
		    }
		}

		/* Получить дробную часть */

		// Если всё ещё есть дробная часть
		if(FloatingPointUnits.Expt < 0){

		    putChar('.');

		    //uint64 Multiplicator = 1;// Мультипликатор содержит множитель, позволяющий домножить число и избавиться от значений после точки

		    /*
		    uint8 MultiplicatorOrder = 0;// Для подсчёта порядка

		    double FloatingPointNumberFraction = packFloatingPoint(FloatingPointUnits);// ??? Надо ли packFloatingPoint
		    double FloatingPointNumberFractionCash = 0;

		    do{
			Multiplicator *= 10;
			MultiplicatorOrder++;

			FloatingPointNumberFractionCash = FloatingPointNumberFraction*Multiplicator;

			FloatingPointUnits = unpackFloatingPoint(FloatingPointNumberFractionCash);

		    }while((FloatingPointUnits.Mtsa & ~((0xFFFFFFFFFFFFFFFFLL)<<(FLOATING_POINT_BIN)(MANTESSE_LENGTH-(FloatingPointUnits.Expt)))) && (MultiplicatorOrder <= MAX_MULTIPLICATOR_ORDER));

		    convertNum2String((FLOATING_POINT_BIN)(Multiplicator), putChar, 10, FALSE);
		    			putChar('\n');
		    			putChar('\r');
		     */

		    // Переместить все дробные цифры в целую часть числа и преобразовать целое в строку
		    convertNum2String((uint64)(packFloatingPoint(FloatingPointUnits)*MULTIPLICATOR_VALUE), putChar, 10, TRUE);
		}

		break;

	    // Особый случай: ноль
	    case FLOATING_POINT_ZERO:

		putChar('0');
		break;

	    // Особый случай: субнормализованное число
	    case FLOATING_POINT_SUBNORMAL:

		putChar('0');
		break;

	    // Особый случай: бесконечность
	    case FLOATING_POINT_INFINITY:

		putChar('i');
		putChar('n');
		putChar('f');
		break;

	    // Особый случай: не число
	    case FLOATING_POINT_NOT_A_NUMBER:

		putChar('n');
		putChar('a');
		putChar('n');
		break;

	    // Неизвестное событие
	    default:

		putChar('?');
	}
}

//----------------------------------------------------------------------------
//  Побитовый доступ к числу с плавающей точкой (на основе кода из: http://ru.wikipedia.org/wiki/Число_одинарной_точности)
//----------------------------------------------------------------------------
PRIVATE FloatingPointUnits_s unpackFloatingPoint(FLOATING_POINT_TYPE FloatingPointNumber){

    union{

	FLOATING_POINT_TYPE Fl;// Сюда помещается вещественное число
	FLOATING_POINT_BIN Bin;// А здесь можно получить его биты
    }FloatingPointStorage;

    // Здесь будут храниться промежуточные результаты
    FloatingPointUnits_s FloatingPointUnits;

    // Проинициализировать структуру
    FloatingPointUnits.Sgn = 0;
    FloatingPointUnits.Expt = 0;
    FloatingPointUnits.Mtsa = 0;

    // Поместить вещественное число в хранилище
    FloatingPointStorage.Fl = FloatingPointNumber;

    // Включена обработка чисел одинарной точности
    #if defined SINGLE_PRECISION

	// Знак
	FloatingPointUnits.Sgn = ( FloatingPointStorage.Bin >> 31 ) ? -1 : 1;

	// Порядок
	FloatingPointUnits.Expt = (int8)((uint32)( FloatingPointStorage.Bin >> 23 ) & 0xFF);
	FloatingPointUnits.Expt -= 127;

	// Мантисса
	FloatingPointUnits.Mtsa = FloatingPointStorage.Bin & 0x7FFFFF;

	// Обработка особых случаев
	switch(FloatingPointUnits.Expt){

	    // Особый случай: 0,-0
	    case EXPONENT_MIN-1:

		if(FloatingPointUnits.Mtsa == 0){

		    FloatingPointUnits.State = FLOATING_POINT_ZERO;
		}
		else{

		    FloatingPointUnits.State = FLOATING_POINT_SUBNORMAL;
		}
		break;

	    // Особый случай: inf, -inf, nan
	    case EXPONENT_MAX+1:

		if(FloatingPointUnits.Mtsa == 0){

		    FloatingPointUnits.State = FLOATING_POINT_INFINITY;
		}
		else{

		    FloatingPointUnits.State = FLOATING_POINT_NOT_A_NUMBER;
		}
		break;

		// Нормальное число
		default:

		    FloatingPointUnits.Mtsa |= (uint32)0x800000;

		    FloatingPointUnits.State = FLOATING_POINT_NORMAL;
	}

    // Включена обработка чисел двойной точности
    #elif defined DOUBLE_PRECISION

	// Знак
	FloatingPointUnits.Sgn = ( FloatingPointStorage.Bin >> 63 ) ? -1 : 1;

	// Порядок
	FloatingPointUnits.Expt = ( FloatingPointStorage.Bin >> 52 ) & 0x07FF;
	FloatingPointUnits.Expt -= 1023;

	// Мантисса
	FloatingPointUnits.Mtsa = FloatingPointStorage.Bin & (uint64)0xFFFFFFFFFFFFFLL;

	// Обработка особых случаев
	switch(FloatingPointUnits.Expt){

	    // Особый случай: 0,-0
	    case EXPONENT_MIN-1:

		if(FloatingPointUnits.Mtsa == 0){

		    FloatingPointUnits.State = FLOATING_POINT_ZERO;
		}

		else{

		    FloatingPointUnits.State = FLOATING_POINT_SUBNORMAL;
		}
		break;

	    // Особый случай: inf, -inf, nan
	    case EXPONENT_MAX+1:

		if(FloatingPointUnits.Mtsa == 0){

		    FloatingPointUnits.State = FLOATING_POINT_INFINITY;
		}
		else{

		    FloatingPointUnits.State = FLOATING_POINT_NOT_A_NUMBER;
		}
		break;

		// Нормальное число
		default:

		    FloatingPointUnits.Mtsa |= (uint64)0x10000000000000LL;

		    FloatingPointUnits.State = FLOATING_POINT_NORMAL;
	}

    #endif

    return FloatingPointUnits;

}

//----------------------------------------------------------------------------
//  Закодировать знак, мантиссу и порядок в вещественный тип данных
//----------------------------------------------------------------------------
PRIVATE FLOATING_POINT_TYPE packFloatingPoint(FloatingPointUnits_s FloatingPointUnits){

    union{

	FLOATING_POINT_TYPE Fl;// Сюда помещается вещественное число
	FLOATING_POINT_BIN Bin;// А здесь можно получить его биты
    }FloatingPointStorage;

    // Проинициализировать хранилище, установив знак числа и обнулив остальные биты
    if(FloatingPointUnits.Sgn == -1) FloatingPointStorage.Fl = -0;
    else FloatingPointStorage.Fl = 0;

    #if defined SINGLE_PRECISION

	// Упаковать в зависимости от состояния
	switch(FloatingPointUnits.State){

	    // Число нормальное
	    case FLOATING_POINT_NORMAL:

		/* Установить порядок// */

		// Избавиться от знака
		FloatingPointUnits += 127;

		// Установить порядок (Магическая операция с числами 8 и 15 вместо сдвига на 23 необходима, чтобы избавиться от warning: left shift count >= width of type, к алгоритму отношения не имеет)
		FloatingPointStorage.Bin |= (FLOATING_POINT_BIN)((int32)(FloatingPointUnits.Expt << 8) << 15);

		/* //Установить порядок */

		// Установить мантиссу
		FloatingPointStorage.Bin |= FloatingPointUnits.Mtsa & 0x7FFFFF;

		break;

	    // Субнормальное (денормализованное) число
	    case FLOATING_POINT_SUBNORMAL:

		// Установить мантиссу
		FloatingPointStorage.Bin |= FloatingPointUnits.Mtsa & 0x7FFFFF;

		break;

	    // Ноль
	    case FLOATING_POINT_ZERO:

		// Установить знак и порядок не требуется, так как это начальное значение

		break;

            // Бесконечность
	    case FLOATING_POINT_INFINITY:

		// Установить максимальный порядок
		FloatingPointStorage.Bin |= 0x7F800000;

		break;

	    // Не число
	    case FLOATING_POINT_NOT_A_NUMBER:

		// Установить максимальный порядок и мантиссу
		FloatingPointStorage.Bin |= (FloatingPointUnits.Mtsa == 0) ? (0x7FFFFFFF) : (0x7F800000 | (FloatingPointUnits.Mtsa & 0x7FFFFF));

		break;

	    // Неизвестный тип (В таком случае вернуть особое состояние: ноль)
	    default:

		// Установить знак и порядок не требуется, так как это начальное значение

		break;
	}

    #elif defined DOUBLE_PRECISION

	// Упаковать в зависимости от состояния
	switch(FloatingPointUnits.State){

	    // Число нормальное
	    case FLOATING_POINT_NORMAL:

		/* Установить порядок// */

		// Избавиться от знака
		FloatingPointUnits.Expt += 1023;

		// Установить порядок (Магическая операция с числами 5 и 47 вместо сдвига на 52 необходима, чтобы избавиться от warning: left shift count >= width of type, к алгоритму отношения не имеет)
		FloatingPointStorage.Bin |= (FLOATING_POINT_BIN)((int64)(FloatingPointUnits.Expt << 5) << 47);

		/* //Установить порядок */

		// Установить мантиссу
		FloatingPointStorage.Bin |= FloatingPointUnits.Mtsa & 0xFFFFFFFFFFFFFLL;

		break;

	    // Субнормальное (денормализованное) число
	    case FLOATING_POINT_SUBNORMAL:

		// Установить мантиссу
		FloatingPointStorage.Bin |= (FloatingPointUnits.Mtsa & 0xFFFFFFFFFFFFFLL);

		break;

	    // Ноль
	    case FLOATING_POINT_ZERO:

		// Установить знак и порядок не требуется, так как это начальное значение

		break;

            // Бесконечность
	    case FLOATING_POINT_INFINITY:

		// Установить максимальный порядок
		FloatingPointStorage.Bin |= 0x7FF0000000000000LL;

		break;

	    // Не число
	    case FLOATING_POINT_NOT_A_NUMBER:

		// Установить число
		FloatingPointStorage.Bin |= (FloatingPointUnits.Mtsa == 0) ?  (0x7FFFFFFFFFFFFFFFLL) : (0x7ff0000000000000LL|(FloatingPointUnits.Mtsa & 0xFFFFFFFFFFFFFLL));

		break;

	    // Неизвестный тип (В таком случае вернуть особое состояние: ноль)
	    default:

		// Установить знак и порядок не требуется, так как это начальное значение

		break;
	}

    #endif

    // Вернуть упакованное число
    return FloatingPointStorage.Fl;

}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
