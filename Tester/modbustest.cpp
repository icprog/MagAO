#include <stdlib.h>
#include <curses.h>
#include "AdamLib.h"
#include "Utils.h"

int main(int argc, char **argv) {

    AdamModbus *adam = new AdamModbus();
    int bitmask;
    int adamvec[8];
    string time0;
    string time1;
    int counter = 0;
    int transId = 0;

    int israndom = false;
    int quit = false;

   
    adam->setLogLevel(Logger::LOG_LEV_TRACE); 

    initscr();
    nodelay(stdscr, true);
    noecho();
    cbreak();

    printw("---- Modbus Test ----\n\n(R)Random multi coils write\n(8..1) Toggle single coil\n(Q) Quit");

    while(!quit) {
        
        if (!adam->ReadInputStatus(1, 0, 8, &bitmask)) {
            mvprintw(8,0,"ReadInputStatus failed, reconnecting");
            adam->clearTcp();
            adam->initTcp();
        }
        else {
            for (int i=0; i<=7; i++) {
                int bit = 1 << i;
                adamvec[i]= (bitmask & bit) ? 1:0;
            }
            mvprintw(9,0,"Adam in status: %d:%d:%d:%d:%d:%d:%d:%d Hex %02X   ", adamvec[7], adamvec[6], adamvec[5], adamvec[4], adamvec[3], adamvec[2], adamvec[1], adamvec[0], bitmask & 0xFF);
        }            

        if (!adam->ReadCoilStatus(1, 16, 8, &bitmask)) {
            mvprintw(8,0,"ReadCoilStatus failed, reconnecting");
            adam->clearTcp();
            adam->initTcp();
        }
        else {
            for (int i=0; i<=7; i++) {
                int bit = 1 << i;
                adamvec[i]=(bitmask & bit) ? 1 :0;
            }
            mvprintw(10,0,"Adam out status: %d:%d:%d:%d:%d:%d:%d:%d Hex %02X   ", adamvec[7], adamvec[6], adamvec[5], adamvec[4], adamvec[3], adamvec[2], adamvec[1], adamvec[0], bitmask & 0xFF);
        }

        int ch, bit, val;
        if ( (ch = getch()) != ERR) {
            switch (toupper(ch)) 
                {
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                    israndom = false;
                    bit = ch - '1';
                    val = (bitmask & 0xFF) & (1<<bit);
                    
                    mvprintw(8,0,"Toggle bit %d                                  ", bit+1);

                    transId = 0;
                    if (adam->SendForceSingleCoil(1,bit + 17, val ? 0 : 1, &transId))
                        mvprintw(6,0,"SendForceSingeCoils Toggle %d Ok (id %04x)", bit+1, transId);
                    else
                        mvprintw(6,0,"SendForceSingeCoils Toggle %d Nok          ", bit+1);
                    if (adam->RecvForceSingleCoil(transId))
                        mvprintw(7,0,"RecvForceSingeCoils Ok                ");
                    else
                        mvprintw(7,0,"RecvForceSingeCoils Nok               ");
                    break;
                case 'R':
                    israndom = true;
                    mvprintw(8,0,"Started random write                             ");
                    break;
                case 'Q':
                    quit = true;
                    break;
                }
        }

        if ((israndom) && (counter == 0) && (!quit)) {
            BYTE val = (BYTE)(random()%256) & 0x6F;
            
            transId = 0;
            if (adam->SendForceMultiCoils(1, 17, 8, 1, &val, &transId))
                mvprintw(6,0,"SendForceMultiCoils (%02X) Ok  (id %04x)   ", val, transId);
            else
                mvprintw(6,0,"SendForceMultiCoils (%02X) Nok            ", val);
                
            if (adam->RecvForceMultiCoils(transId)) 
                mvprintw(7,0,"SendForceMultiCoils Ok                   ");
            else
                mvprintw(7,0,"SendForceMultiCoils Nok                  ");

        }

        msleep(500);

        counter = (counter+1)%3;

    }
    endwin();

    return 0;
}
