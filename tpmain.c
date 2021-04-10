#include "tp.h"
char *getComando(char *linhaComando)
{
  int tam = strlen(linhaComando);
  char *comando = (char *)malloc(sizeof(char) * STRINGS_SIZE);
  printf("%d\n", tam);
  for (int i = 0; i < tam; i++)
  {
    if (linhaComando[i] != ' ')
    {
      comando[i] = linhaComando[i];
    }
    else
    {
      comando[i] = "\0";
      tam = strlen(comando);
      printf("%s\n", comando);
      printf("%d\n", tam);
      getchar();
      return comando;
    }
  }
  return comando;
}

int main()
{

  int carregado = 0; // variavel 0-fat não carregada e 1-para fat carregada
  char *linhaComando = (char *)malloc(sizeof(char) * STRINGS_SIZE);
  char *comando = (char *)malloc(sizeof(char) * STRINGS_SIZE);
  char *parametros = (char *)malloc(sizeof(char) * STRINGS_SIZE);
  do
  {
    strcpy(linhaComando, "");
    strcpy(comando, "");
    strcpy(parametros, "");
    printf("$");
    gets(linhaComando);
    fflush(stdin);
   // comando = getComando(linhaComando);
    separaString(linhaComando,comando,parametros," ");
    if (carregado == 1 || strcmp(comando, "load") == 0 || strcmp(comando, "init") == 0 || strcmp(comando, "exit") == 0)
    {
      if (strcmp(comando, "init") == 0)
      {
        carregado = init();
      }
      else if (strcmp(comando, "load") == 0)
      {
        carregado = load();
      }
      else if (strcmp(comando, "ls") == 0)
      {
        ls(parametros);
      }
      else if (strcmp(comando, "mkdir") == 0)
      {
        mkdir(parametros);
      }
      else if (strcmp(comando, "create") == 0)
      {
        create(parametros);
      }
      else if (strcmp(comando, "unlink") == 0)
      {
        unlink(parametros);
      }
      else if (strcmp(comando, "write") == 0)
      {
        write(parametros);
      }
      else if (strcmp(comando, "append") == 0) // mkdir pasta criar
      {
         append(parametros);
      }
      else if (strcmp(comando, "read") == 0)
      {
        read(parametros);
      }
      else if (strcmp(comando, "exit") == 0)
      {
        printf("exit\n");
        free(linhaComando);
        free(comando);
        return 0;
      }
      else
      {
        printf("comando invalido\n");
      }
    }
    else
    {
      printf("ERRO fat nao inicializada!\n");
    }
  } while (1);
}
