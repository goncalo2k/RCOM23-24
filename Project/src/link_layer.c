// Link layer protocol implementation

#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////




int llopen(LinkLayer connectionParameters)
{
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0)
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

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused - Neste caso não se espera tempo nenhum por outro byte
    newtio.c_cc[VMIN] = 0;  // Blocking read until 5 chars received - Não queremos ficar em blocking

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    //printf("New termios structure set\n");

    return 0;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    int bytes = write(fd, buf, bufSize);
    printf("%d bytes written\n", bytes);

    // Wait until all bytes have been written to the serial port
    sleep(1);

    return bufSize;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    
    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
   // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 0;
}


int byteStuffing(unsigned char *buf, int bufSize) {
    if (buf == NULL || bufSize < 1) {
        printf("Invalid buffer\n");
        return -1;
    }

    //Init copy buffer
    unsigned char *copyBuf[bufSize];

    for (int i = 0; i < bufSize; i++) {
        if (buf[i] == FLAG) {
            copyBuf[i] = ESC;
            copyBuf[i+1] = FLAG ^ 0x20;
        } else if (buf[i] == ESC) {
            continue;
        } else {
            copyBuf[i] = buf[i];
        }
    }
    return 0;
}

int byteDestuffing(unsigned char *buf, int bufSize) {
    if (buf == NULL || bufSize < 1) {
        printf("Invalid buffer\n");
        return -1;
    }

    //Init copy buffer
    unsigned char *copyBuf[bufSize];

    for (int i = 0; i < bufSize; i++) {
        if (buf[i] == FLAG) {
            copyBuf[i] = ESC;
            copyBuf[i+1] = FLAG ^ 0x20;
        } else if (buf[i] == ESC) {
            continue;
        } else {
            copyBuf[i] = buf[i];
        }
    }

    return 0;
}