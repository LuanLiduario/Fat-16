#include "tp.h"
char *getComando(char *linhaComando)
{
  int tam = strlen(linhaComando);
  char *comando = (char *)malloc(sizeof(char) * 50);
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

void printDir()
{
  printf("$");
}

int main()
{

  int carregado = 0; // variavel 0-fat não carregada e 1-para fat carregada
  char *linhaComando = (char *)malloc(sizeof(char) * 50);
  char *comando = (char *)malloc(sizeof(char) * 50);
  char *parametros = (char *)malloc(sizeof(char) * 50);
  do
  {
    strcpy(linhaComando, "");
    strcpy(comando, "");
    strcpy(parametros, "");
    printDir();
    gets(linhaComando);
    fflush(stdin);
   // comando = getComando(linhaComando);
    separaString(linhaComando,comando,parametros);
    printf("%s\n", comando);

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
        printf("ls\n");
      }
      else if (strcmp(comando, "mkdir") == 0)
      {
        printf("mkdir\n");
      }
      else if (strcmp(comando, "create") == 0)
      {
        printf("create\n");
      }
      else if (strcmp(comando, "unlink") == 0)
      {
        printf("unlink\n");
      }
      else if (strcmp(comando, "write") == 0)
      {
        printf("write\n");
      }
      else if (strcmp(comando, "append") == 0)  mkdir pasta criar
      {
        printf("append\n");
      }
      else if (strcmp(comando, "read") == 0)
      {
        printf("read\n");
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
