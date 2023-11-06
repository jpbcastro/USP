//Esta aplicação é um jogo da velha usando sockets e threads

#include <stdio.h>
#include <string.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

int meusocket;											//declaração do socket
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;		//inicialização do mutex
struct sockaddr_in addr;								//struct usada para conexões na internet
void * enviar();										//função para enviar jogada ao servidor
void * receber();										//função para receber o estado ddo jogo no servidor
int done=1;												//variável para identificar se o jogo acabou
char jogo[5][6];										//armazenara o a tabela do jogo da velha
void new_game();										//atribui os parametros iniciais para a tabela
void print_jogo();										//imprime a tabela do jogo

int main(){
	meusocket = socket(AF_INET,SOCK_STREAM,0);			//inicialização do socket TCP	
	if(meusocket==-1){
		printf("Erro ao criar socket");
		return 1;
	}
	addr.sin_family = AF_INET;							//familia de protocolo usada
	addr.sin_port = htons(1300);						//numero daa porta
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");		//IP do host
	memset(&addr.sin_zero,0,sizeof(addr.sin_zero));		//zera os bits não utillizados

	printf("Conectando...\n");
	if(connect(meusocket,(struct sockaddr*)&addr,sizeof(addr)) == -1){	//conecta cliente ao socket
  		printf("Erro ao se conectar!\n");
  		return 1;
  	}
  	printf("Conectado\n");

  	new_game();
  	printf("\nDigite o numero da casa que você queira jogar ou Q para sair\n");
  	print_jogo();

    pthread_t threads[2];											//declaração de duas threads
    void *status;
    pthread_attr_t attr;											//endereço das threads
    pthread_attr_init(&attr);										//inicializa os atributos das threads apontadas por attr
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);	//determina a thread criada em joinable state

    pthread_create(&threads[0], &attr, enviar, NULL);				//thread responsavel por enviar dados ao servidor
    pthread_create(&threads[1], &attr, receber, NULL);				//thread responsavel por receber dados do servidor 

    while(done){}

    pthread_mutex_destroy(&mutex);		//destroi o mutex
	close(meusocket);					//fecha o socket

	return 0;
}

void *enviar(){
	char mensagem[2];
	while(done){ 
	  fgets(mensagem,2,stdin);							//espera o jogador inserir um comando
	  mensagem[1] = '\0';
	  if(strcmp(mensagem,"q")==0){						//caso o comando seja Q o jogador sai do jogo
	    done=0;
	  }
	  send(meusocket,mensagem,strlen(mensagem),0);		//envia o comando do jogador para o servidor
	}
	  pthread_exit(NULL);         						//encerra o thread caso o jogador escolha sair do jogo
}

void *receber(){
	while(done){
	  recv(meusocket,jogo,sizeof(jogo),0);		//recebe do servidor a tabela do jogo atualizada
	  print_jogo();								//imprime na tela a tabela do jogo
	  if(strcmp(jogo[0],"PLAYE")==0){			//caso alguem tenha ganhado o jogo se encerra
	  	done=0;
	  }
	} 
	  pthread_exit(NULL);						//encerra a thread 
}

void print_jogo(){
	int i;
	printf("\n");
	for(i=0;i<5;i++){
		printf("%s\n", jogo[i]);
	}
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
