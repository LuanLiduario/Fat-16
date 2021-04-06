#include "tp.h"

char* getComando(char * linhaComando){
    int tam = strlen(linhaComando);
    char *comando = (char*) malloc(sizeof(char)*50);
    printf("%d\n",tam);
    for(int i = 0; i < tam; i++)
    {
      if(linhaComando[i] != ' ')
      {
          comando[i] = linhaComando[i];
      }else{
          comando[i] = "\0"; 
          return comando;
      }
    }
    return comando;
}

int main() {
 
   int carregado = 0; // variavel 0-fat não carregada e 1-para fat carregada
   char * linhaComando = (char*) malloc(sizeof(char)*50);
   char * comando = (char*) malloc(sizeof(char)*50);
   do{
      printf("$");
      gets(linhaComando);
      comando = getComando(linhaComando);
      printf("%s\n",comando );
      if(strcmp(comando,"load") == 0 || strcmp(comando,"init") == 0 || carregado == 1){
          if(strcmp(comando,"init") == 0){
            init();
            carregado = 1;
          }else if(strcmp(comando,"load") == 0){
            printf("load\n");
             carregado = 1;
          }else if(strcmp(comando,"ls"){

          }else if(strcmp(comando,"mkdir"){
            
          }else if(strcmp(comando,"create"){
            
          }else if(strcmp(comando,"unlink"){
            
          }else if(strcmp(comando,"write"){
            
          }else if(strcmp(comando,"append"){
            
          }else if(strcmp(comando,"read"){
            
          }else if(strcmp(comando,"read"){
            
          }else {
            printf("comando invalido\n");
          }
         
      }else{
        printf("ERRO fat nao inicializada ou carregada\n");
      }

   }while(1);
   init();
   return 0;
 }


