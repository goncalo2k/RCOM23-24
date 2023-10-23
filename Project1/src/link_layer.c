// Link layer protocol implementation
#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

int fd;
struct termios oldtio;
struct termios newtio;

LinkLayer params;

int currSeqNumber = 0;

int alarmEnabled = FALSE;
int alarmCount = 0;

void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;
    printf("Alarm num: %d\n", alarmCount);
}

void sendResponseFrame(uint8_t control)
{
    uint8_t buf[] = {FLAG, ADDRESS_SEN, control, ADDRESS_SEN ^ control, FLAG};
    write(fd, buf, 5);
}

uint8_t receiveControlFrame()
{
    uint8_t state = S_START, byte, control, address;

    do
    {
        read(fd, &byte, 1);
        switch (state)
        {
        case S_START:
            if (byte == FLAG)
                state = S_FLAG_RECEIVED;
            break;
        case S_FLAG_RECEIVED:
            if (byte == ADDRESS_SEN)
            {
                state = S_A_RECEIVED;
                address = byte;
            }

            else if (byte == FLAG)
                state = S_FLAG_RECEIVED;
            break;
        case S_A_RECEIVED:
            if (byte == SET || byte == UA || byte == DISC ||
                byte == RR0 || byte == RR1 || byte == REJ0 ||
                byte == REJ1 || byte == I0 || byte == I1)
            {
                state = S_C_RECEIVED;
                control = byte;
            }
            else if (byte == FLAG)
            {
                state = S_FLAG_RECEIVED;
            }
            break;
        case S_C_RECEIVED:
            if (byte == (address ^ control))
                state = S_BCC_OK;
            else if (byte == FLAG)
            {
                state = S_FLAG_RECEIVED;
            }
            break;
        case S_BCC_OK:
            if (byte == FLAG)
            {
                state = S_STOP;
            }
            break;
        case S_STOP:
            break;
        default:
            state = S_START;
        }
    } while (state != S_STOP);

    return control;
}

void sendControlFrame(uint8_t control, uint8_t response)
{
    uint8_t buf[] = {FLAG, ADDRESS_SEN, control, ADDRESS_SEN ^ control, FLAG};

    alarmCount = 0;
    alarmEnabled = FALSE;

    while (alarmCount < params.nRetransmissions)
    {
        if (!alarmEnabled)
        {
            write(fd, buf, 5);
            alarm(params.timeout); // Set alarm to be triggered in X seconds
            alarmEnabled = TRUE;
        }

        if (receiveControlFrame() == response)
        {
            alarm(0);
            break;
        }
    }

    if (alarmCount == params.nRetransmissions)
    {
        printf("Failed to send frame\n");
    }
}
////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    params = connectionParameters;
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);

    if (fd == -1)
    {
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    if (connectionParameters.role == LlTx)
    {
        sendControlFrame(SET, UA);
        printf("Sent SET and received UA\n");
    }
    else
    {
        uint8_t ans;
        do
        {
            ans = receiveControlFrame();
        } while (ans != SET);

        printf("Received SET\n");
        sendResponseFrame(UA);
        printf("Sent UA\n");
    }

    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    printf("Sending packet (llwrite)...\n");
    uint8_t frame[bufSize * 2], xor = 0;
    int i, j = 0;

    frame[j++] = FLAG;
    frame[j++] = ADDRESS_SEN;
    frame[j++] = currSeqNumber ? I1 : I0;
    frame[j++] = ADDRESS_SEN ^ frame[2];

    for (i = 0; i < bufSize; i++)
    {
        if (buf[i] == FLAG)
        {
            frame[j++] = ESC;
            frame[j++] = FLAG ^ 0x20;
        }
        else if (buf[i] == ESC)
        {
            frame[j++] = ESC;
            frame[j++] = ESC ^ 0x20;
        }
        else
        {
            frame[j++] = buf[i];
        }
        xor ^= buf[i];
    }

    frame[j++] = xor;
    frame[j++] = FLAG;

    alarmCount = 0;
    alarmEnabled = FALSE;

    while (alarmCount < params.nRetransmissions)
    {
        if (!alarmEnabled)
        {
            write(fd, frame, j);
            alarm(params.timeout); // Set alarm to be triggered in X seconds
            alarmEnabled = TRUE;
        }

        if (receiveControlFrame() == (currSeqNumber ? RR0 : RR1))
        {
            alarm(0);
            break;
        }
    }

    if (alarmCount == params.nRetransmissions)
    {
        printf("Failed to send frame\n");
        return -1;
    }

    currSeqNumber = 1 - currSeqNumber;

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    printf("Waiting for packet (llread)...\n");
    uint8_t state = S_START, byte, control, address;
    int i = 0;

    do
    {
        read(fd, &byte, 1);
        switch (state)
        {
        case S_START:
            if (byte == FLAG)
                state = S_FLAG_RECEIVED;
            break;
        case S_FLAG_RECEIVED:
            if (byte == ADDRESS_SEN)
            {
                state = S_A_RECEIVED;
                address = byte;
            }

            else if (byte == FLAG)
                state = S_FLAG_RECEIVED;
            break;
        case S_A_RECEIVED:
            if (byte == SET || byte == UA || byte == DISC ||
                byte == RR0 || byte == RR1 || byte == REJ0 ||
                byte == REJ1 || byte == I0 || byte == I1)
            {
                state = S_C_RECEIVED;
                control = byte;
            }
            else if (byte == FLAG)
            {
                state = S_FLAG_RECEIVED;
            }
            break;
        case S_C_RECEIVED:
            if (byte == (address ^ control))
                state = S_BCC_OK;
            else if (byte == FLAG)
            {
                state = S_FLAG_RECEIVED;
            }
            break;
        case S_BCC_OK:
            if (byte == FLAG)
            {
                state = S_STOP;
            }
            else if (byte == ESC)
            {
                state = S_ESCAPED;
            }
            else
            {
                packet[i++] = byte;
                state = S_DATA_RECEIVED;
            }
            break;
        case S_DATA_RECEIVED:
            if (byte == FLAG)
            {
                state = S_STOP;
            }
            else if (byte == ESC)
            {
                state = S_ESCAPED;
            }
            else
            {
                packet[i++] = byte;
            }
            break;
        case S_ESCAPED:
            if (byte == ESCD_FLAG)
            {
                packet[i++] = FLAG;
                state = S_DATA_RECEIVED;
            }
            else if (byte == ESCD2_FLAG)
            {
                packet[i++] = ESC;
                state = S_DATA_RECEIVED;
            }
            else
            {
                state = S_START;
            }
            break;
        case S_STOP:
            break;
        default:
            state = S_START;
        }
    } while (state != S_STOP);

    // check bcc2
    uint8_t check_bcc2 = 0;
    for (int j = 0; j < i - 1; j++)
    {
        check_bcc2 ^= packet[j];
    }

    if (check_bcc2 != packet[i - 1])
    {
        printf("BCC2 error\n");
        sendResponseFrame(currSeqNumber ? REJ0 : REJ1);
        return -1;
    }

    sendResponseFrame(currSeqNumber ? RR0 : RR1);
    currSeqNumber = 1 - currSeqNumber;

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    uint8_t control;
    if (params.role == LlTx)
    {
        sendControlFrame(DISC, UA);
    }
    else
    {
        do
        {
            control = receiveControlFrame();
        } while (control != DISC);

        sendResponseFrame(DISC);

        do
        {
            control = receiveControlFrame();
        } while (control != UA);
    }

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 1;
}
