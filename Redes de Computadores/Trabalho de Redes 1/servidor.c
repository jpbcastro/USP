//Esta aplicação é um jogo da velha usando sockets e threads

#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

int meusocket;											//declaração do socket servidor
int socket_cliente1, socket_cliente2;					//declaração dos sockets clientes
struct sockaddr_in addr;								//struct usada para conexões na internet
struct sockaddr_in addr_cliente1;						//estrutura onde sera guardado o endereço do cliente 1
struct sockaddr_in addr_cliente2;						//estrutura onde sera guardado o endereço do cliente 2
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  	//inicialização do mutex
void *enviar();											//função para enviar o jogo para os clientes
void *escutar1();										//função para escutar o cliente 1
void *escutar2();										//função para escutar o cliente 2
int done=1;												//variavel para desligar o servidor
int fim1=1;												//variavel para identificar se o jogador 1 saiu
int fim2=1;												//variavel para identificar se o jogador 2 saiu
int flag=0;												//flag para identificar se o servidor recebeu um dado novo
char jogo[5][6];										//armazenara o a tabela do jogo da velha
void muda_tabela(char *resposta, int jogador);			//atualiza a tabela do jogo com a ultima jogada
void new_game();										//atribui os parametros iniciais para a tabela
int verificar_vitoria();								//verifica se alguem ganhou
void ganhou(char player);								//sinaliza aos jogadores o jogador que ganhou e encerra o jogo
int turno =1;											//sinaliza de qual jogador é a vez


int main(){
	meusocket = socket(AF_INET,SOCK_STREAM,0);			//inicialização do socket TCP
	if(meusocket == -1){
	  printf("Erro ao criar socket\n");
	  return 1;
	 }

	addr.sin_family      = AF_INET;						//familia de protocolo usada
	addr.sin_port        = htons(1300);					//numero daa porta
	addr.sin_addr.s_addr = INADDR_ANY;					//aceita conecções com qualquer IP da maquina
	memset(&addr.sin_zero,0,sizeof(addr.sin_zero));		//zera os bits não utillizados

	if(bind(meusocket,(struct sockaddr*)&addr,sizeof(addr)) == -1){		//associa o servidor a um IP e uma porta
	  printf("Erro na funcao bind()\n");
	  return 1;
	  }

	if(listen(meusocket,2) == -1){				//coloca o socket no modo de escuta para estabelecer 2 novas conecções
	  printf("Erro na funcao listen()\n");
	  return 1;
	  }

	printf("\nAguardando jogadores...\n");

	int a = sizeof(addr);															//tamanho da estrutura do endereço
	socket_cliente1 = accept(meusocket,(struct sockaddr*) &addr_cliente1, &a);		//aceita conexão com o cliente 1 e guarda seu endereço em adddr_cliente1
	socket_cliente2 = accept(meusocket,(struct sockaddr*) &addr_cliente2, &a);		//aceita conexão com o cliente 2 e guarda seu endereço em adddr_cliente2

	if(socket_cliente1 == -1){
	  printf("Cliente 1 não foi aceito\n");
	  return 1;
	}

	if(socket_cliente2 == -1){
	  printf("Cliente 2 não foi aceito\n");
	  return 1;
	}

	new_game();

	printf("\nJogadores conectados!\n");
	pthread_t threads[3];												//declara 3 threads
	    void *status;
	    pthread_attr_t attr;                                            //endereço das threads
	    pthread_attr_init(&attr);                                       //inicializa os atributos das threads apontadas por attr
	    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);	//determina a thread criada em joinable state

	    pthread_create(&threads[0], &attr, enviar, NULL);			//thread responsavel por enviar os dados aos clientes
	    pthread_create(&threads[1], &attr, escutar1, NULL);			//thread responsavel por receber dados do cliente 1
	    pthread_create(&threads[2], &attr, escutar2, NULL);			//thread responsavel por receber dados do cliente 2


	while(done){				//loop que verifica se os jogadores sairam, em caso afirmativo, finaliza o programa
		if(fim1==0 && fim2==0){
			done=0;
		}
	}

	pthread_mutex_destroy(&mutex);		//destroi o mutex
	close(meusocket);					//fecha o socket

	return 0;
}

void *enviar(){
	while(done){ 
		if(flag){											//caso tenha recebido um dado novo
			send(socket_cliente1,jogo,sizeof(jogo),0);		//envia a tabela atualizadda para jogador 1
			send(socket_cliente2,jogo,sizeof(jogo),0);		//envia a tabela atualizadda para jogador 2
			flag=0;
		}
	}
}


void *escutar1(){
	char resposta[2];
	while(fim1){
	  recv(socket_cliente1,resposta,2,0);              // Recebe mensagem do cliente
	  resposta[1] = '\0';
	  if(turno==1){ 									//caso seja o turno desse jogador
		  printf("\n Cliente1: %s\n",resposta);			
		  muda_tabela(resposta, 1);						//atualiza a tabela com a nova marcação do jogador
		  int vitoria = verificar_vitoria();			//verifica se houve uma vitoria	
		  if(vitoria){
		  	ganhou('1');								//atribui vitoria ao jogador 1
		  }
		  flag=1;										//sinaliza que há dado novo para a função enviar
		  turno=2;										//passa o turno para o jogador 2
	  }
	  if(strcmp(resposta,"q")==0){						//verifica se o input do jogador foi de sair do jogo
   		 fim1=0;										//sinaliza o fim dessa thread
  	  }
	}
		pthread_exit(NULL);								//finaliza a thread
}

void *escutar2(){
	char resposta[2];
	while(fim2){
	  recv(socket_cliente2,resposta,2,0);           // Recebe mensagem do cliente 
	  resposta[1] = '\0';
	  if(turno==2){									//caso seja o turno desse jogador
		  printf("\n Cliente2: %s\n",resposta);
		  muda_tabela(resposta, 2);					//atualiza a tabela com a nova marcação do jogador
		  int vitoria = verificar_vitoria();		//verifica se houve uma vitoria	
		  if(vitoria){
		  	ganhou('2');							//atribui vitoria ao jogador 2
		  }
		  flag=1;									//sinaliza que há dado novo para a função enviar
		  turno=1;									//passa o turno para o jogador 1
	  }
	  if(strcmp(resposta,"q")==0){					//verifica se o input do jogador foi de sair do jogo
   		 fim2=0;									//sinaliza o fim dessa thread
  	  }
	}
		pthread_exit(NULL);							//finaliza a thread
}

void muda_tabela(char *resposta, int jogador){
	char r;
	char caso = resposta[0];
	if(jogador==1){
		r = 'x';
	}
	if(jogador==2){
		r = 'o';
	}
	switch(caso){
		case '1':
			jogo[0][0] = r;
			break;
		case '2':
			jogo[0][2] = r;
			break;
		case '3':
			jogo[0][4] = r;
			break;
		case '4':
			jogo[2][0] = r;
			break;
		case '5':
			jogo[2][2] = r;
			break;
		case '6':
			jogo[2][4] = r;
			break;
		case '7':
			jogo[4][0] = r;
			break;
		case '8':
			jogo[4][2] = r;
			break;
		case '9':
			jogo[4][4] = r;
			break;
	}
}

int verificar_vitoria(){
	int vitoria = 0;
	int i;
	for(i=0;i<5;i=i+2){
		if(jogo[i][0]==jogo[i][2] && jogo[i][0]==jogo[i][4]){
			vitoria=1;
		}
	}
	for(i=0;i<5;i=i+2){
		if(jogo[0][i]==jogo[2][i] && jogo[0][i]==jogo[4][i]){
			vitoria=1;
		}
	}
	if(jogo[0][0]==jogo[2][2] && jogo[0][0]==jogo[4][4]){
		vitoria=1;
	}
	if(jogo[2][2]==jogo[0][4] && jogo[2][2]==jogo[4][0]){
		vitoria=1;
	}
	return vitoria;
}

void ganhou(char player){
	strcpy(jogo[0],"PLAYE");
	strcpy(jogo[1],"R----");
	strcpy(jogo[2],"-----");
	strcpy(jogo[3],"GANHO");
	strcpy(jogo[4],"U----");
	jogo[2][0]=player;
}

void new_game(){
	int i;
	for(i=1;i<4;i=i+2){
		strcpy(jogo[i],"-----");
	}
	strcpy(jogo[0],"1|2|3");
	strcpy(jogo[2],"4|5|6");
	strcpy(jogo[4],"7|8|9");
}
