/* 
 * File:   main.cpp
 * Author: lion
 *
 * Created on January 6, 2015, 12:32 PM
 */

#include <cstdlib>
#include <cstdio>
#include <cassert>

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
   FILE *fout = fopen("/dev/ylmfdev", "w");
   assert(NULL != fout);
   if (NULL != fout) {
      fprintf(fout, "Hello!\n");
      fclose(fout);

      FILE *fin = fopen("/dev/ylmfdev", "r");
      int c;
      while ((c = fgetc(fin)) != EOF)
      {
         printf("%c", (char)c);
      }
      fclose(fin);
   }
   return 0;
}

