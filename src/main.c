/**
 * @author Steffen Breinbjerg
 * @date 20201027
 * @brief CRC16 calculation on received message from the
 * IM871A WMBUS-dongle.
 *
 *Extract from WMBUS_HCL_Spec_V1.6.pdf -> (2.3.8):
 *The CRC computation starts from the Control Field and ends with the last octet of
 *the Payload Field or Time Stamp Field or RSSI Field.
 * Looking at the dataframe the above sentence means:
 * The first byte Start of frame(a5) and the last two bytes FCS field (Checksum field) are removed
 * and the calculation is done on the remaining bytes.
 *
 *
 *IM871A uses CRC16-CCITT Polynomial
 *G(x) = 1 + x^5 + x^12 + x^16
 *The Polynomial can be represented in a 16bit int
 *1000 1000 0001 0001
 *The IM871A Uses the Reversed version
 *0010 0000 1101 1000 -> Which gives this hexadecimal representation: 0x8408
 */


/** Include files */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/** Defines */
#define CRC16_POLY 0x8408
#define CRC16_INIT_VALUE 0xFFFF



/** Test messages */
uint8_t payload[8][85] = {{"820327442d2c5768663230028d20cb103407201d82040f26a7e808ff3449f9e9d2e4b28bd7e7c6f1c6df"},
                          {"820327442d2c5768663230028d20cd12340720519df247ff65e751662a300bc4e5c67da86477f0182637"},
                          {"820327442d2c5768663230028d206972dd032089aa2c0a75352edf4b64a7b908470ba6171c89e52aab8a"},
                          {"820327442d2c5768663230028d2086f0dd0320368763cbae145d5f6c56d0afad5f369db1e22a7e6311df"},
                          {"820327442d2c5768663230028d2076b0dd0320c872a70560f4faef03685bcac1ac8fca34cb3ef0dbacf1"},
                          {"820327442d2c5768663230028d2079b3dd032072e1bc19a9d337d17a731fcea7733abcaa002ca6e33478"},
                          {"820327442d2c5768663230028d207ed0dd032008a6f44c44320b0e636f694819e91b2a5f2fb1dc753191"},
                          {"820327442d2c5768663230028d2083e1dd0320d4e65337143ba7621f5ebf580642d40fb7d66c45dd4e19"}};

uint8_t long_payload[] = "82032d442d2c5768663230028d207cc2dd0320f8325c5952304521c530f237b6ee19e4cd7d6778f660152192a4751a46";

/** expected results: */
/** 3885 - CRC 0*/
/** c1ab - CRC 1 */
/** b7ac - CRC 2 */
/** e2af - CRC 3 */
/** 229e - CRC 4 */
/** 02f3 - CRC 5 */
/** a0ee - CRC 6 */
/** b64f - CRC 7 */
/** f667 - Long payload CRC */



typedef unsigned char BYTE;

/** Table to speed up the calculation */
const uint16_t CRC16_Table[] =
        {0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
         0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
         0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
         0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
         0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
         0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
         0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
         0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
         0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
         0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
         0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
         0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
         0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
         0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
         0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
         0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
         0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
         0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
         0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
         0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
         0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
         0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
         0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
         0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
         0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
         0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
         0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
         0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
         0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
         0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
         0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
         0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78};

/** Prototypes */
uint16_t CRC16_Calc(uint8_t *, uint16_t, uint16_t);

uint16_t CRC16_Calc_no_table(uint8_t *, uint16_t, uint16_t);

uint16_t CRC16_original(uint8_t *, uint16_t, uint16_t);

int main() {
    uint16_t calc = 0;
    /** Sample run */
    for (int i = 0; i < 8; i++) {
        calc = ~CRC16_Calc(payload[i], strlen(payload[i]), CRC16_INIT_VALUE);
        printf("Calculated checksum[%d]:\t\t\t\t %x\n", i, calc);
        calc = ~CRC16_Calc_no_table(payload[i], strlen(payload[1]), CRC16_INIT_VALUE);
        printf("Calculated checksum[%d](No Table):\t %x\n", i, calc);
    }
    calc = ~CRC16_Calc(long_payload, strlen(long_payload), CRC16_INIT_VALUE);
    printf("Calculated checksum(long payload):\t\t\t %x\n", calc);
    calc = ~CRC16_Calc_no_table(long_payload, strlen(long_payload), CRC16_INIT_VALUE);
    printf("Calculated checksum(Long payload)(No Table): %x\n", calc);
    return 0;
}


/**
 * CRC16_Calc and CRC16_Calc_no_table are both working as intended.
 */


uint16_t CRC16_Calc(uint8_t *data, uint16_t length, uint16_t initVal) {
    BYTE Data;
    uint8_t hx1, hx2;
    uint16_t crc = initVal;
    length /= 2;
// iterate over all bytes
    while (length--) {
        hx1 = (*data >= 'A') ? (*data - 'A' + 10) : (*data - '0');
        data++;
        hx2 = (*data >= 'A') ? (*data - 'A' + 10) : (*data - '0');
        data++;
        Data = (hx1 << 4) & 0xFF;
        Data |= hx2 & 0xF;
// calc new crc
        crc = (crc >> 8) ^ CRC16_Table[(crc ^ Data) & 0x00FF];
    }
// return result
    return crc;
}


// calculate CRC16 without table
uint16_t CRC16_Calc_no_table(uint8_t *data, uint16_t length, uint16_t initVal) {
// init crc
    BYTE Data;
    uint8_t hx1, hx2;
    uint16_t crc = initVal;
    length /= 2;
// iterate over all bytes
    while (length--) {
        int bits = 8;
        hx1 = (*data >= 'A') ? (*data - 'A' + 10) : (*data - '0');
        data++;
        hx2 = (*data >= 'A') ? (*data - 'A' + 10) : (*data - '0');
        data++;
        Data = (hx1 << 4) & 0xFF;
        Data |= hx2 & 0xF;
// iterate over all bits per byte
        while (bits--) {
            if ((Data & 1) ^ (crc & 1)) {
                crc = (crc >> 1) ^ CRC16_POLY;
            } else
                crc >>= 1;
            Data >>= 1;
        }
    }
// return result
    return crc;
}

/** Don't touch - Dark magic is happening here */
uint16_t CRC16_original(uint8_t *data, uint16_t length, uint16_t initVal) {
// init crc
    uint16_t crc = initVal;
// iterate over all bytes
    while (length--) {
        int bits = 8;
        uint8_t byte = *data++;
// iterate over all bits per byte
        while (bits--) {
            if ((byte & 1) ^ (crc & 1)) {
                crc = (crc >> 1) & CRC16_POLY;
            } else
                crc >>= 1;
            byte >>= 1;
        }
    }
// return result
    return crc;
}