#include "AdamTestLib.h"

#include <readline/readline.h>
#include <readline/history.h>


void help() {
   printf("Usage: adamcmd <ipaddr> <adam_number (default=1)\n");
   printf("\n");
   printf("Available commands:\n");
   printf("\n");
   printf("reset bcu\n");
   printf("select user\n");
   printf("select default\n");
   printf("clear fpga\n");
   printf("disable coils\n");
   printf("enable coils\n");
   printf("disable tss\n");
   printf("enable tss\n");
   printf("disable main power\n");
   printf("enable main power\n");
   printf("help\n");
   printf("\n");
}
       
int main(int argc, char **argv) {

   string num="1";
   char *line;

    if ((argc != 2) && (argc != 3)) {
       help();
       exit(0);
    }

    if (argc == 3)
       num = argv[2];

    printf("Connecting to adam %s (n. %s)\n", argv[1], num.c_str());
    Adam *adam = new Adam(argv[1], num, true);

    using_history();
    while ((line = readline("AdOpt>"))) {

     if (strlen(line) == 0)
              continue;
     add_history(line);

     if (strcmp(line, "reset bcu")==0) adam->resetBcu();
     else if (strcmp(line, "select user")==0) adam->selectUser();
     else if (strcmp(line, "select default")==0) adam->selectDefault();
     else if (strcmp(line, "clear fpga")==0) adam->clearFpga();
     else if (strcmp(line, "disable coils")==0) adam->disableCoils();
     else if (strcmp(line, "enable coils")==0) adam->enableCoils();
     else if (strcmp(line, "disable tss")==0) adam->disableTss();
     else if (strcmp(line, "enable tss")==0) adam->enableTss();
     else if (strcmp(line, "disable main power")==0) adam->disableMainPower();
     else if (strcmp(line, "enable main power")==0) adam->enableMainPower();
     else if (strcmp(line, "help")==0) help();
     else printf("Unknown command. Type \"help\" for help\n");

     free(line);

   }

}

