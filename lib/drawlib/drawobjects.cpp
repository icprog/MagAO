
#include "drawobjects.h"

drawobject *drawobject::create(string s) {

   drawobject *obj = NULL;
   int pos = s.find("TYPE ");
   string type = s.substr(pos+5, 4);
   if (type.compare("LINE") == 0)
      obj = new line();
   if (type.compare("CIRC") == 0)
      obj = new circle();
   if (type.compare("ARRO") == 0)
      obj = new arrow();
   if (obj)
      obj->import(s);
   return obj;
}

void sendObject( char *clientName, drawobject *object) {

   char filename[FILENAME_MAX];

   sprintf(filename, "/tmp/%s_objects", clientName);
   FILE *fp;
   if ((fp = fopen(filename, "a")) != NULL) {
      fprintf(fp,"%s\n", object->export2().c_str());
      fclose(fp);
   }
}

// Call multiple times until it returns NULL

// Queste due si possono reimplementare usando messaggi MSGD,
// con l'aggiunta di fare broadcast a più clienti usando un prefisso.

drawobject *getObject( char *clientName) {
   
   static ifstream myfile;
   bool opened = false;
   char filename[FILENAME_MAX];

   if (!myfile.is_open()) {
      sprintf(filename, "/tmp/%s_objects", clientName);
      myfile.open(filename);
      if (!myfile.is_open())
         return NULL;
      opened = true;
   }

   while (!myfile.eof()) {
      string s;
      getline(myfile, s);
      if (s.size()>0) {
         drawobject *obj = drawobject::create(s);
         if (obj)
            if (obj->isValid())
               return obj;
      }
   }

   if (myfile.eof()) {
      myfile.close();

      // Check if we arrived here without reeturning anything!
      // Either because the file was empty, or because all objects wheren't valid
      // Remove the file.
      if (opened)
         unlink(filename);
   }

   return NULL;

}


      

