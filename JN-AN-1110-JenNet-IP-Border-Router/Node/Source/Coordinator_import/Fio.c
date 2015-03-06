/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include "Fio.h"

#include "Config.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* �������� ������������ ��� �������������� ����� ����� %f � Fio_printf() */

// ����������� ������� ����� � ��������� ������
#if defined SINGLE_PRECISION

    #define EXPONENT_MAX 	127// ������������ �������� �������
    #define EXPONENT_MIN 	-126// ����������� �������� �������
    #define EXPONENT_LENGTH 	7// ����� ������� (���)
    #define MANTESSE_LENGTH 	23// ����� �������� (���)

    #define FLOATING_POINT_TYPE float// ���, ������������ ��� ������������� ����� ��������� ��������
    #define FLOATING_POINT_BIN 	uint32// ���, ������������ ��� ���������� �������� � ������ �����

#elif defined DOUBLE_PRECISION

    #define EXPONENT_MAX 	1023// ������������ ��������, ����������� ��������
    #define EXPONENT_MIN 	-1022// ����������� ��������, ����������� ��������
    #define EXPONENT_LENGTH 	11// ����� ������� (���)
    #define MANTESSE_LENGTH 	52// ����� �������� (���)

    #define FLOATING_POINT_TYPE double// ���, ������������ ��� ������������� ����� ������� ��������
    #define FLOATING_POINT_BIN 	uint64// ���, ������������ ��� ���������� �������� � ������ �����

#endif

#define MULTIPLICATOR_VALUE  1000000000000000000LL// ���������, ����������� ��������� ����� � ���������� �� �������� ����� �����
#define MULTIPLICATOR_ORDER  18// ������� ��������������� <2^64 = 18446744073709551616 -> 19 � �������� ������� ����� ����� ����������� double*10^Multiplicator// ������������ ������ (� ������ ������ ���������� ������������� double)// ������������ �������� �����

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

// ��������� � ������� ����� ��������� ����� � ��������� ������
typedef enum{

    FLOATING_POINT_NORMAL = 0,// ����� ��-��������� ����������
    FLOATING_POINT_SUBNORMAL,
    FLOATING_POINT_ZERO,
    FLOATING_POINT_INFINITY,
    FLOATING_POINT_NOT_A_NUMBER,

} FloatingPointState_e;

// ������������ �������������� ������������� �����
typedef struct{

    FloatingPointState_e State;// ������ ��������� � ������� ��������� �����

    int8 Sgn;// ���� ����� � ��������� �������
    int16 Expt;// ������� �����

    FLOATING_POINT_BIN Mtsa;// ��� ��������

} FloatingPointUnits_s;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE void convertBcd2String(Bcd_t CurrentBcd, void (*putChar)(char c));// ������������� BCD � ������
PRIVATE void convertNum2String(FLOATING_POINT_BIN Number, void (*putChar)(char c), uint8 Base, bool isRealNumberConversion);// ������������� ����� � ������
PRIVATE void convertFloatingPoint2String(FLOATING_POINT_TYPE FloatingPointNumber, void (*putChar)(char c));// ������������� ������������ � ������

PRIVATE FloatingPointUnits_s unpackFloatingPoint(FLOATING_POINT_TYPE FloatingPointNumber);// ����������� ����� // ������ ����, ������� �������� � ���������� ���������
PRIVATE FLOATING_POINT_TYPE packFloatingPoint(FloatingPointUnits_s FloatingPointUnits);// ���������� ����� // �������� ������������ ������ ��������� � �������� ������� ������, ���� �� ���������� �����

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
//  ��������� �����
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
// �������� ������� �������������� ����� � ������
//----------------------------------------------------------------------------
PUBLIC void Fio_convertNum2String(uint64 Number, void (*putChar)(char c), uint8 Base){

    convertNum2String((FLOATING_POINT_BIN)Number, putChar, Base, FALSE);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

PRIVATE void convertBcd2String(Bcd_t CurrentBcd, void (*putChar)(char c)){

    uint8 i = 0;

    //-- ����� �������� ������ ������ ������ ���� ��� ������ ����� � ������ ����� ������� �����

    // ���������� ���������� ����� ������ �����
    while(((CurrentBcd & (0xF<<(i*4))) == 0) && (i < BCD_FRACTION_LENGTH)){

	i++;
    };

    if(CurrentBcd < 0){

	putChar('-');
	CurrentBcd *= -1;
    }

    // ����� ��� ������� �����
    if((i-BCD_FRACTION_LENGTH) >= 0){

	// �������� �� ������� ����� �����
	CurrentBcd >>= 4*BCD_FRACTION_LENGTH;

	// ������� �����
	convertNum2String(CurrentBcd, putChar, 16, FALSE);
    }
    // ���� ������� �����
    else{

	union{

	    Bcd_t Cash;
	    uint8 Collector[BCD_WHOLE_LENGTH+BCD_FRACTION_LENGTH];
	}Bcd;// ����������� ��� ���������� ������ ���� �����

	Bcd.Cash = CurrentBcd;

	//-- �������� ����� ����� �����
	uint8 k;
	for(k=0; k<((BCD_WHOLE_LENGTH+BCD_FRACTION_LENGTH)/2); k++){

	    if(k >= (BCD_WHOLE_LENGTH/2)) Bcd.Collector[k] = 0;
	}

	//-- �������� �� ������� ����� �����
	Bcd.Cash >>= 4*BCD_FRACTION_LENGTH;

	//-- ������� ����� �����
	convertNum2String(Bcd.Cash, putChar, 16, FALSE);

	putChar('.');

	Bcd.Cash = CurrentBcd;

	//-- �������� ������� ������
	for(k=0; k<((BCD_WHOLE_LENGTH+BCD_FRACTION_LENGTH)/2); k++){

	    if(k < (BCD_WHOLE_LENGTH/2)) Bcd.Collector[k] = 0;
	}

	Bcd.Cash >>= i*4;// ������ ������ ����

	//-- ���������� ���������� ����� ����� ��������� ������� � ������� �����// ��� �����, ��������, ��� ��� 0.0021 - ��� ����
	uint8 zeros = 0;
	for(k=0; k<(BCD_FRACTION_LENGTH - i); k++){

	    if((Bcd.Cash & 0xF<<(k*4)) == 0){

		zeros++;
	    }
	}

	//-- ������� �������������� ����
	for(k=0; k<zeros; k++){

	    putChar('0');
	}

	convertNum2String(Bcd.Cash , putChar, 16, FALSE);// ������� �����
    }
}

//----------------------------------------------------------------------------
// ��������� ������� ��� �������������� � ������, ����� ������������ ��� �������������� ������������� �����
//----------------------------------------------------------------------------
PRIVATE void convertNum2String(FLOATING_POINT_BIN Number, void (*putChar)(char c), uint8 Base, bool isRealNumberConversion){

    // ����� ������ ��������
    #define BUF_LENGTH 33

    // �����, ���� ���������� ������
    char buf[BUF_LENGTH];

    // ������� ��������� �� �������� ������
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

    // ������� �������������� ��������� ��� ������ �� ������� ���� �����
    char* UpperLimitingPointer = buf + BUF_LENGTH;

    // ��������� ���� �����
    uint8 FixedZerosWindow = 0;
    uint8 FixedNinesWindow = 0;

    // ���� ������������ ��� �������������� ������� ����� ������������� �����, �� ������ ����������� ������ ����� (����� ������ ���� ����� ������ ������ ����� � ���������)
    if(isRealNumberConversion){

	// ������������ �����
	if(((buf + BUF_LENGTH - 1) - p) < MULTIPLICATOR_ORDER){

	    uint8 i;
	    char* pcash = p;

	    for(i=0; i < MULTIPLICATOR_ORDER - ((buf + BUF_LENGTH - 1) - pcash); i++){

		*--p = '0';
	    }
	}

	// ���������, ������������ ��� ������
	char* SearchingPointer = p;

	// ����� ���� ����� ������������ � �������������� ������
	while(*SearchingPointer && (FixedZerosWindow <= MAX_FRACTION_DIGITS) &&  (FixedNinesWindow <= MAX_FRACTION_DIGITS)){

	    if(*SearchingPointer == '0'){

		// ���� ������������ ������ ���� ��������� �����
		if(FixedZerosWindow == 0) UpperLimitingPointer = SearchingPointer;

		// ���������������� �������� ������ ����
		FixedZerosWindow++;
	    }
	    else{

		// ���� �����������, �� �� ���������, �������� ������� ������ ����
		FixedZerosWindow = 0;
	    }

	    if(*SearchingPointer == '9'){

		// ���� ������������� ������ �����, ��������� �����
		if(FixedNinesWindow == 0) UpperLimitingPointer = SearchingPointer;

		// ���������������� �������� ������ ����
		FixedNinesWindow++;
	    }
	    else{

		// ���� �����������, �� �� ���������, �������� ������� ������ ����
		FixedNinesWindow = 0;
	    }
	    SearchingPointer++;
	}

	// ����� ����� �������� ����������, ��� ������������ ���������� �������� ������ ������� �����������


	// ���������� ����� ����� ������, ���� ��������� �������� ���� �������� �����
	if (FixedZerosWindow > MAX_FRACTION_DIGITS){


	    *UpperLimitingPointer = '\0';
	}

	// ���������� ����� ����� ������, ���� ��������� �������� ���� � ��������� �� ������� ���������� �����
	if (FixedNinesWindow > MAX_FRACTION_DIGITS){

	    // ���� ����� ������ ������
	    if(UpperLimitingPointer == p){

		// ���������� �� ��� ����� �������������� �����
		if (*UpperLimitingPointer == '9') *UpperLimitingPointer++ = '1';
		else *UpperLimitingPointer += 1;

		// ����� ���������� ����� ������
		*UpperLimitingPointer = '\0';

	    // ���� ����������� �������
	    }else if(UpperLimitingPointer > p){

		// ���������� ����� ������
		*(UpperLimitingPointer--) = '\0';

		// ��������� ���������� ����� �� �������
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
//  ������������� ������������ � ������
//----------------------------------------------------------------------------
PRIVATE void convertFloatingPoint2String(FLOATING_POINT_TYPE FloatingPointNumber, void (*putChar)(char c)){

	// ��������� ����������� � ��������� ����� � ���������� �������
	if(ABS(FloatingPointNumber) > MAX_INTEGER_VALUE){

	    // ���� �� ����������� ����� �� �������
	    putChar('?');
	    return;
	}

	// ����������� �����
	FloatingPointUnits_s FloatingPointUnits = unpackFloatingPoint(FloatingPointNumber);

	// ������� ���� �����
	if(FloatingPointUnits.Sgn < 0) putChar('-');

	// ������� �������� � ����������� � ����� �����
	switch(FloatingPointUnits.State){

	    // ���������� �����
	    case FLOATING_POINT_NORMAL:

		/* �������� ����� ����� ����� */

		// ���� ����� ����� ����� �����
		if(FloatingPointUnits.Expt >= 0){

		    // �������� ��������, ������� ����� �����, ������������ ��� ����������� ������� �������������� ����� �����
		    convertNum2String((FLOATING_POINT_BIN)(FloatingPointUnits.Mtsa>>(MANTESSE_LENGTH-FloatingPointUnits.Expt)), putChar, 10, FALSE);

		    // ������ �� �������� ����� �����
		    FloatingPointUnits.Mtsa &= ~((0xFFFFFFFFFFFFFFFFLL)<<(FLOATING_POINT_BIN)(MANTESSE_LENGTH-(FloatingPointUnits.Expt)));

		    // ���������, �������� �� � �������� ������� �����, ����� ����������� �������
		    if(FloatingPointUnits.Mtsa != 0){

			uint8 Idx = 0;
			uint64 Shift = 0xFFFFFFFFFFFFFFFFLL;
			uint64 MtsaCash = FloatingPointUnits.Mtsa;

			// ���������� ������� ��� � �������� �������� ������� �����
			while(MtsaCash & (Shift << Idx++));

			// ����������� ������� ��� ������� �����
			FloatingPointUnits.Expt = FloatingPointUnits.Expt - (54 - Idx);

			// �������� ������� ����� � ������ ��������
			FloatingPointUnits.Mtsa <<= 54 - Idx;
		    }
		    // ���� �������� �� �� ����� ���� -> ������� ����
		    else{

			putChar('.');
			putChar('0');
		    }
		}

		/* �������� ������� ����� */

		// ���� �� ��� ���� ������� �����
		if(FloatingPointUnits.Expt < 0){

		    putChar('.');

		    //uint64 Multiplicator = 1;// �������������� �������� ���������, ����������� ��������� ����� � ���������� �� �������� ����� �����

		    /*
		    uint8 MultiplicatorOrder = 0;// ��� �������� �������

		    double FloatingPointNumberFraction = packFloatingPoint(FloatingPointUnits);// ??? ���� �� packFloatingPoint
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

		    // ����������� ��� ������� ����� � ����� ����� ����� � ������������� ����� � ������
		    convertNum2String((uint64)(packFloatingPoint(FloatingPointUnits)*MULTIPLICATOR_VALUE), putChar, 10, TRUE);
		}

		break;

	    // ������ ������: ����
	    case FLOATING_POINT_ZERO:

		putChar('0');
		break;

	    // ������ ������: ������������������ �����
	    case FLOATING_POINT_SUBNORMAL:

		putChar('0');
		break;

	    // ������ ������: �������������
	    case FLOATING_POINT_INFINITY:

		putChar('i');
		putChar('n');
		putChar('f');
		break;

	    // ������ ������: �� �����
	    case FLOATING_POINT_NOT_A_NUMBER:

		putChar('n');
		putChar('a');
		putChar('n');
		break;

	    // ����������� �������
	    default:

		putChar('?');
	}
}

//----------------------------------------------------------------------------
//  ��������� ������ � ����� � ��������� ������ (�� ������ ���� ��: http://ru.wikipedia.org/wiki/�����_���������_��������)
//----------------------------------------------------------------------------
PRIVATE FloatingPointUnits_s unpackFloatingPoint(FLOATING_POINT_TYPE FloatingPointNumber){

    union{

	FLOATING_POINT_TYPE Fl;// ���� ���������� ������������ �����
	FLOATING_POINT_BIN Bin;// � ����� ����� �������� ��� ����
    }FloatingPointStorage;

    // ����� ����� ��������� ������������� ����������
    FloatingPointUnits_s FloatingPointUnits;

    // ������������������� ���������
    FloatingPointUnits.Sgn = 0;
    FloatingPointUnits.Expt = 0;
    FloatingPointUnits.Mtsa = 0;

    // ��������� ������������ ����� � ���������
    FloatingPointStorage.Fl = FloatingPointNumber;

    // �������� ��������� ����� ��������� ��������
    #if defined SINGLE_PRECISION

	// ����
	FloatingPointUnits.Sgn = ( FloatingPointStorage.Bin >> 31 ) ? -1 : 1;

	// �������
	FloatingPointUnits.Expt = (int8)((uint32)( FloatingPointStorage.Bin >> 23 ) & 0xFF);
	FloatingPointUnits.Expt -= 127;

	// ��������
	FloatingPointUnits.Mtsa = FloatingPointStorage.Bin & 0x7FFFFF;

	// ��������� ������ �������
	switch(FloatingPointUnits.Expt){

	    // ������ ������: 0,-0
	    case EXPONENT_MIN-1:

		if(FloatingPointUnits.Mtsa == 0){

		    FloatingPointUnits.State = FLOATING_POINT_ZERO;
		}
		else{

		    FloatingPointUnits.State = FLOATING_POINT_SUBNORMAL;
		}
		break;

	    // ������ ������: inf, -inf, nan
	    case EXPONENT_MAX+1:

		if(FloatingPointUnits.Mtsa == 0){

		    FloatingPointUnits.State = FLOATING_POINT_INFINITY;
		}
		else{

		    FloatingPointUnits.State = FLOATING_POINT_NOT_A_NUMBER;
		}
		break;

		// ���������� �����
		default:

		    FloatingPointUnits.Mtsa |= (uint32)0x800000;

		    FloatingPointUnits.State = FLOATING_POINT_NORMAL;
	}

    // �������� ��������� ����� ������� ��������
    #elif defined DOUBLE_PRECISION

	// ����
	FloatingPointUnits.Sgn = ( FloatingPointStorage.Bin >> 63 ) ? -1 : 1;

	// �������
	FloatingPointUnits.Expt = ( FloatingPointStorage.Bin >> 52 ) & 0x07FF;
	FloatingPointUnits.Expt -= 1023;

	// ��������
	FloatingPointUnits.Mtsa = FloatingPointStorage.Bin & (uint64)0xFFFFFFFFFFFFFLL;

	// ��������� ������ �������
	switch(FloatingPointUnits.Expt){

	    // ������ ������: 0,-0
	    case EXPONENT_MIN-1:

		if(FloatingPointUnits.Mtsa == 0){

		    FloatingPointUnits.State = FLOATING_POINT_ZERO;
		}

		else{

		    FloatingPointUnits.State = FLOATING_POINT_SUBNORMAL;
		}
		break;

	    // ������ ������: inf, -inf, nan
	    case EXPONENT_MAX+1:

		if(FloatingPointUnits.Mtsa == 0){

		    FloatingPointUnits.State = FLOATING_POINT_INFINITY;
		}
		else{

		    FloatingPointUnits.State = FLOATING_POINT_NOT_A_NUMBER;
		}
		break;

		// ���������� �����
		default:

		    FloatingPointUnits.Mtsa |= (uint64)0x10000000000000LL;

		    FloatingPointUnits.State = FLOATING_POINT_NORMAL;
	}

    #endif

    return FloatingPointUnits;

}

//----------------------------------------------------------------------------
//  ������������ ����, �������� � ������� � ������������ ��� ������
//----------------------------------------------------------------------------
PRIVATE FLOATING_POINT_TYPE packFloatingPoint(FloatingPointUnits_s FloatingPointUnits){

    union{

	FLOATING_POINT_TYPE Fl;// ���� ���������� ������������ �����
	FLOATING_POINT_BIN Bin;// � ����� ����� �������� ��� ����
    }FloatingPointStorage;

    // ������������������� ���������, ��������� ���� ����� � ������� ��������� ����
    if(FloatingPointUnits.Sgn == -1) FloatingPointStorage.Fl = -0;
    else FloatingPointStorage.Fl = 0;

    #if defined SINGLE_PRECISION

	// ��������� � ����������� �� ���������
	switch(FloatingPointUnits.State){

	    // ����� ����������
	    case FLOATING_POINT_NORMAL:

		/* ���������� �������// */

		// ���������� �� �����
		FloatingPointUnits += 127;

		// ���������� ������� (���������� �������� � ������� 8 � 15 ������ ������ �� 23 ����������, ����� ���������� �� warning: left shift count >= width of type, � ��������� ��������� �� �����)
		FloatingPointStorage.Bin |= (FLOATING_POINT_BIN)((int32)(FloatingPointUnits.Expt << 8) << 15);

		/* //���������� ������� */

		// ���������� ��������
		FloatingPointStorage.Bin |= FloatingPointUnits.Mtsa & 0x7FFFFF;

		break;

	    // ������������� (�����������������) �����
	    case FLOATING_POINT_SUBNORMAL:

		// ���������� ��������
		FloatingPointStorage.Bin |= FloatingPointUnits.Mtsa & 0x7FFFFF;

		break;

	    // ����
	    case FLOATING_POINT_ZERO:

		// ���������� ���� � ������� �� ���������, ��� ��� ��� ��������� ��������

		break;

            // �������������
	    case FLOATING_POINT_INFINITY:

		// ���������� ������������ �������
		FloatingPointStorage.Bin |= 0x7F800000;

		break;

	    // �� �����
	    case FLOATING_POINT_NOT_A_NUMBER:

		// ���������� ������������ ������� � ��������
		FloatingPointStorage.Bin |= (FloatingPointUnits.Mtsa == 0) ? (0x7FFFFFFF) : (0x7F800000 | (FloatingPointUnits.Mtsa & 0x7FFFFF));

		break;

	    // ����������� ��� (� ����� ������ ������� ������ ���������: ����)
	    default:

		// ���������� ���� � ������� �� ���������, ��� ��� ��� ��������� ��������

		break;
	}

    #elif defined DOUBLE_PRECISION

	// ��������� � ����������� �� ���������
	switch(FloatingPointUnits.State){

	    // ����� ����������
	    case FLOATING_POINT_NORMAL:

		/* ���������� �������// */

		// ���������� �� �����
		FloatingPointUnits.Expt += 1023;

		// ���������� ������� (���������� �������� � ������� 5 � 47 ������ ������ �� 52 ����������, ����� ���������� �� warning: left shift count >= width of type, � ��������� ��������� �� �����)
		FloatingPointStorage.Bin |= (FLOATING_POINT_BIN)((int64)(FloatingPointUnits.Expt << 5) << 47);

		/* //���������� ������� */

		// ���������� ��������
		FloatingPointStorage.Bin |= FloatingPointUnits.Mtsa & 0xFFFFFFFFFFFFFLL;

		break;

	    // ������������� (�����������������) �����
	    case FLOATING_POINT_SUBNORMAL:

		// ���������� ��������
		FloatingPointStorage.Bin |= (FloatingPointUnits.Mtsa & 0xFFFFFFFFFFFFFLL);

		break;

	    // ����
	    case FLOATING_POINT_ZERO:

		// ���������� ���� � ������� �� ���������, ��� ��� ��� ��������� ��������

		break;

            // �������������
	    case FLOATING_POINT_INFINITY:

		// ���������� ������������ �������
		FloatingPointStorage.Bin |= 0x7FF0000000000000LL;

		break;

	    // �� �����
	    case FLOATING_POINT_NOT_A_NUMBER:

		// ���������� �����
		FloatingPointStorage.Bin |= (FloatingPointUnits.Mtsa == 0) ?  (0x7FFFFFFFFFFFFFFFLL) : (0x7ff0000000000000LL|(FloatingPointUnits.Mtsa & 0xFFFFFFFFFFFFFLL));

		break;

	    // ����������� ��� (� ����� ������ ������� ������ ���������: ����)
	    default:

		// ���������� ���� � ������� �� ���������, ��� ��� ��� ��������� ��������

		break;
	}

    #endif

    // ������� ����������� �����
    return FloatingPointStorage.Fl;

}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
