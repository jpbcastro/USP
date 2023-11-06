#include <string>
#include <bitset>
#include <iostream>
#include <vector>
#include <array>
using namespace std;

struct bloco{			//bloco que possuirá um caractere em bits, a paridade para controle de erro por paridade,
	bitset<8> byte;		//	e o resto para controle de erro por CRC
	int paridade;
	bitset<32> R;
};

int TipoDeControleDeErro = 0;		//determina qual será o controle de erro, 0 para paridade par, 1 para paridade impar, 2 para CRC

void AplicacaoTransmissora();									//função para receber a mensagem
void CamadaDeAplicacaoTransmissora(string mensagem);			//função para transformar os caracteres da mensagem em bits
void CamadaEnlaceDadosTransmissora(vector<bitset<8>> quadro);	//direciona para qual controle de erro será usado

void CamadaEnlaceDadosTransmissoraControleDeErroBitParidadePar(vector<bitset<8>> quadro);	//recebe os bits da mensagem, calcula os bits de paridade
void CamadaEnlaceDadosTransmissoraControleDeErroBitParidadeImpar(vector<bitset<8>> quadro);	//	e armazena esses valores em um vetor de structs "blocos"
void CamadaEnlaceDadosTransmissoraControleDeErroCRC(vector<bitset<8>> quadro);				//recebe os bits da mensagem, calcula o resto, com o gerador 0x04C11DB7 do CRC-32
																							//	e armazena esses valores em um vetor de structs "blocos"
void MeioDeComunicacao(bloco *fluxoDeBits, int tamanho);				//recebe o vetor de blocos e altera alguns bits da mensagem
void CamadaEnlaceDadosReceptora(bloco *fluxoDeBits, int tamanho);		//direciona para qual verificação de erro será utilizada

void CamadaEnlaceDadosReceptoraControleDeErroBitParidadePar(bloco *fluxoDeBits, int tamanho);	//para cada setor de 8 bits(1 caractere) calcula novamente o bit de paridade,
void CamadaEnlaceDadosReceptoraControleDeErroBitParidadeImpar(bloco *fluxoDeBits, int tamanho);	//	caso o novo bit de paridade seja diferente do antigo significa que ocorreu erro
void CamadaEnlaceDadosReceptoraControleDeErroCRC(bloco *fluxoDeBits, int tamanho);				//verifica se d+r é divisivel pelo gerador, caso não seja, significa que ocorreu erro

void CamadaDeAplicacaoReceptora(bloco *fluxoDeBits, int tamanho);	//recebe um vetor de "blocos", retira o vetor de binarios e transforma em um vetor de chars 
void AplicacaoReceptora(vector<char> mensagem);						//imprime o vetor de chars

vector<bitset<8>> chartobin(string mensagem);			//função para transformar chars em sequências de 8 bits
vector<char> bintochar(vector<bitset<8>> binarios);		//função para transformar sequências de 8 bits em chars

int main(){
	AplicacaoTransmissora();
	return 0;
}

void AplicacaoTransmissora(void){				//função para receber a mensagem
	string mensagem;							//string para armazenar a mensagem
	cout<<"Digite uma mensagem:"<<endl;
	cin>>mensagem;								//recebe a mensagem
	CamadaDeAplicacaoTransmissora(mensagem);
}

void CamadaDeAplicacaoTransmissora(string mensagem){	//deve transformar uma string em um vetor de sequências de 8 bits
	vector<bitset<8>> quadro; 							//declara um vetor de sequências de 8 bits chamada "quadro"
	quadro = chartobin(mensagem);						//"quadro" recebe um vetor de sequências de 8 bits correspondentes aos caracteres de mensagem
	CamadaEnlaceDadosTransmissora(quadro);
}

void CamadaEnlaceDadosTransmissora(vector<bitset<8>> quadro){		//direciona para qual controle de erro será usado
	switch(TipoDeControleDeErro){
		case 0:
			CamadaEnlaceDadosTransmissoraControleDeErroBitParidadePar(quadro);		//paridade par
			break;
		case 1:
			CamadaEnlaceDadosTransmissoraControleDeErroBitParidadeImpar(quadro);	//paridade impar
			break;
		case 2:
			CamadaEnlaceDadosTransmissoraControleDeErroCRC(quadro);					//CRC
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CamadaEnlaceDadosTransmissoraControleDeErroBitParidadePar(vector<bitset<8>> quadro){	//controle de erro por paridade par
	auto n = quadro.size();		//"n" recebe o tamanho do vetor "quadro"
	bloco fluxoDeBits[n];		//cria um vetor estatico de structs "bloco" para armazenar a paridade e os 8 bits de um caractere
	int i, j;
	int cont;					//contador para a quantidade de 1 que cada byte (sequência de 8 bits) possui
	for(i=0; i<n; i++){			//percorre os blocos do vetor
		cont=0;
		fluxoDeBits[i].byte = quadro[i];		//armazena os 8 bits na struct
		for(j=0; j<8; j++){						//percorre os bits do byte do bloco
			if(fluxoDeBits[i].byte[j]==1){		//conta quantos 1 os 8 bits possui
				cont++;
			}
		}
		if(cont%2==0){
			fluxoDeBits[i].paridade=0;		//caso o número de 1 seja par, adota o valor de paridade 0 para esse elemento do vetor
		}
		else{
			fluxoDeBits[i].paridade=1;		//caso o número de 1 seja impar, adota o valor de paridade 1 para esse elemento do vetor
		}
	}
	MeioDeComunicacao(fluxoDeBits, n);		//envia o vetor de "blocos" e seu tamanho;
}

void CamadaEnlaceDadosTransmissoraControleDeErroBitParidadeImpar(vector<bitset<8>> quadro){	//controle de erro por paridade impar
	auto n = quadro.size();		//"n" recebe o tamanho do vetor "quadro"
	bloco fluxoDeBits[n];		//cria um vetor estatico de structs "bloco" para armazenar a paridade e os 8 bits de um caractere
	int i, j;
	int cont;					//contador para a quantidade de 1 que cada byte (sequência de 8 bits) possui
	for(i=0; i<n; i++){			//percorre os blocos do vetor
		cont=0;
		fluxoDeBits[i].byte = quadro[i];		//armazena os 8 bits na struct
		for(j=0; j<8; j++){						//percorre os bits do byte do bloco
			if(fluxoDeBits[i].byte[j]==1){		//conta quantos 1 os 8 bits possui
				cont++;
			}
		}
		if(cont%2==0){
			fluxoDeBits[i].paridade=1;		//caso o número de 1 seja par, adota o valor de paridade 1 para esse elemento do vetor
		}
		else{
			fluxoDeBits[i].paridade=0;		//caso o número de 1 seja impar, adota o valor de paridade 0 para esse elemento do vetor
		}
	}
	MeioDeComunicacao(fluxoDeBits, n);		//envia o vetor de "blocos" e seu tamanho;
}

void CamadaEnlaceDadosTransmissoraControleDeErroCRC(vector<bitset<8>> quadro){
	auto n = quadro.size();				//calcula o tamanho dos bits de mensagem
	bloco fluxoDeBits[n];				//cria a estrutura para o fluxo da menssagem
	int numbaux, r;						//numbaux será utilizado para armazenar numericamente a menssagem e r é valor em int do resto
	int i, j;
	for(i=0; i<n; i++){
		fluxoDeBits[i].byte = quadro[i];				//armazena o bits de dados no fluxo de mensagem
		numbaux = (int)(quadro[i].to_ulong());			//armazena numericamente a messagem para gerar o resto
		r = (numbaux*2^32) % (2^33+0x04C11DB7);			//calcula o valor do resto a ser enviado junto com os dados	
		fluxoDeBits[i].R = r;							//guarda no fluxo o resto a ser enviado junto com os dados

	}
	MeioDeComunicacao(fluxoDeBits, n);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void MeioDeComunicacao(bloco *fluxoDeBits, int tamanho){		//deve alterar os bits da mensagem
	int i, j, porcentagemDeErros;
	porcentagemDeErros = 5;			//porcentagemDeErros é o número em % que detemina se o erro ocorrerá ou não
	for(i=0; i<tamanho; i++){		//percorre os blocos do vetor
		for(j=0; j<8; j++){			//percorre os bits do byte do bloco
			if((rand()%100)<porcentagemDeErros){		//se o número randomico de 0 a 99 for menor que a porcentagemDeErros, ocorre o erro
				fluxoDeBits[i].byte[j]==0?fluxoDeBits[i].byte[j]=true:fluxoDeBits[i].byte[j]=false;		//o valor do bit é trocado
			}
		}
	}
	CamadaEnlaceDadosReceptora(fluxoDeBits, tamanho);
}

void CamadaEnlaceDadosReceptora(bloco *fluxoDeBits, int tamanho){		//direciona para qual verificação de erro será realizada
	switch(TipoDeControleDeErro){
		case 0:
			CamadaEnlaceDadosReceptoraControleDeErroBitParidadePar(fluxoDeBits, tamanho);	//verificação de erro por bit de paridade par
			break;
		case 1:
			CamadaEnlaceDadosReceptoraControleDeErroBitParidadeImpar(fluxoDeBits, tamanho);	//verificação de erro por bit de paridade impar
			break;
		case 2:
			CamadaEnlaceDadosReceptoraControleDeErroCRC(fluxoDeBits, tamanho);				//verificação de erro por CRC
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CamadaEnlaceDadosReceptoraControleDeErroBitParidadePar(bloco *fluxoDeBits, int tamanho){	//verifica se houve corrupção nos bits, se houve, substitui o char por #
	bitset<8> byteaux (string("00100011"));	//bit do char #
	int i, j, paridaux;						//"paridaux"(paridade auxiliar) será a nova paridade
	int cont;								//contador para a quantidade de 1 que cada byte (sequência de 8 bits) possui
	for(i=0; i<tamanho; i++){				//percorre os blocos do vetor	
		cont=0;
		for(j=0; j<8; j++){					//percorre os bits do byte do bloco
			if(fluxoDeBits[i].byte[j]==1){	//conta quantos 1 os 8 bits possui
				cont++;
			}
		}
		if(cont%2==0){						//caso o número de 1 seja par, adota o valor 0 para "paridaux"
			paridaux=0;
		}
		else{
			paridaux=1;						//caso o número de 1 seja impar, adota o valor 1 para "paridaux"
		}
		if(paridaux!=fluxoDeBits[i].paridade){	//caso "paridaux" seja diferente do bit de paridade do byte,
			fluxoDeBits[i].byte=byteaux;		//	os bits desse bloco são igualados ao bit do caracter #
		}
	}
	CamadaDeAplicacaoReceptora(fluxoDeBits, tamanho);
}

void CamadaEnlaceDadosReceptoraControleDeErroBitParidadeImpar(bloco *fluxoDeBits, int tamanho){	//verifica se houve corrupção nos bits, se houve, substitui o char por #
	bitset<8> byteaux (string("00100011"));	//bit do char #
	int i, j, paridaux;						//"paridaux"(paridade auxiliar) será a nova paridade
	int cont;								//contador para a quantidade de 1 que cada byte (sequência de 8 bits) possui
	for(i=0; i<tamanho; i++){				//percorre os blocos do vetor	
		cont=0;
		for(j=0; j<8; j++){					//percorre os bits do byte do bloco
			if(fluxoDeBits[i].byte[j]==1){	//conta quantos 1 os 8 bits possui
				cont++;
			}
		}
		if(cont%2==0){						//caso o número de 1 seja par, adota o valor 1 para "paridaux"
			paridaux=1;
		}
		else{
			paridaux=0;						//caso o número de 1 seja impar, adota o valor 0 para "paridaux"
		}
		if(paridaux!=fluxoDeBits[i].paridade){	//caso "paridaux" seja diferente do bit de paridade do byte,
			fluxoDeBits[i].byte=byteaux;		//	os bits desse bloco são igualados ao bit do caracter #
		}
	}
	CamadaDeAplicacaoReceptora(fluxoDeBits, tamanho);
}

void CamadaEnlaceDadosReceptoraControleDeErroCRC(bloco *fluxoDeBits, int tamanho){
	bitset<8> byteaux (string("00100011")); //bit do char #
	int i, r, d;

	for(i=0; i<tamanho; i++){
		r = (int)(fluxoDeBits[i].R.to_ulong());				//gera numericamente o valor do resto para a verificação de erro 
		d = ((int)(fluxoDeBits[i].byte.to_ulong()))*2^32;	//gera numericamente o valor dos dados para a verificação de erro
		if((2^33+0x04C11DB7)%(d+r)!=0){
			fluxoDeBits[i].byte=byteaux;					//guarda a mensagem a ser enviada no caso de erro
		}

	}

	CamadaDeAplicacaoReceptora(fluxoDeBits, tamanho);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CamadaDeAplicacaoReceptora(bloco *fluxoDeBits, int tamanho){	//recebe o vetor de blocos e deve retornar um vetor de chars
	vector<bitset<8>> quadro;						//declara o vetor de sequência de 8 bits para armazenar os bits de caractere do "bloco"
	int i;
	for(i=0; i<tamanho; i++){
		quadro.push_back(fluxoDeBits[i].byte);		//copia os bytes(sequência de 8 bits) do vetor de blocos para o novo vetor de bytes
	}
	vector<char> mensagem;							//declara o vetor de char que possuirá a mensagem
	mensagem = bintochar(quadro);					//o vetor de sequências de 8 bits é transformado em um vetor de caracteres e armazenado em "mensagem"
	AplicacaoReceptora(mensagem);
}

void AplicacaoReceptora(vector<char> mensagem){		//recebe o vetor de caracteres e o imprime na tela
	cout << "A mensagem recebida foi:";
	int i;
	auto n = mensagem.size();		//armazena o tamanho da mensagem
	for(i=0; i<n; i++){				//imprime a mensagem
    	cout << mensagem[i];
    }
}

vector<bitset<8>> chartobin(string mensagem){		//transforma uma string "mensagem" em um vetor de segmentos de 8 bits "binarios"
	int n = mensagem.length();						//"n" armazena o tamanho da mensagem
	int numero[n];									//"numero" armazenará os inteiros correspondentes, em ascii, aos caracteres da "mensagem"
	vector<bitset<8>> binarios;						//"binarios" armazenará os segmentos de 8 bits correspondente aos inteiros de "numero"
	bitset<8> byte;									//byte para auxilar a colocar os segmentos de 8 bits no vetor "binarios"
	int i;
	for(i=0; i<n; i++){								//percorre a mensagem
		numero[i]=mensagem[i];						//transforma o caractere em inteiro da tabela ascii
		byte = numero[i];							//transforma o inteiro em um segmento de 8 bits
		binarios.push_back(byte);					//armazena o segmento de 8 bits desse caractere no vetor "binarios"
	}
	return binarios;								//retorna o vetor "binarios"
}

vector<char> bintochar(vector<bitset<8>> binarios){		//transforma um vetor de segmentos de 8 btis "binarios" em um vetor de caracteres "chars"
	auto n = binarios.size();							//armazena o tamanho do vetor "binarios"
	vector<int> numbers;								//vetor de inteiros correspondente ao vetor de segmento de 8 bits
	int numbaux, i;										//"numbaux" é um inteiro para auxiliar a colocar os valores no vetor "numbers"
	for(i=0; i<n; i++){									//percorre o vetor binários
		numbaux = (int)(binarios[i].to_ulong());		//transforma um segmento de 8 bits em um inteiro
		numbers.push_back(numbaux);						//adiciona o inteiro ao vetor "numbers" 
	}
	vector<char> chars{};								//vetor para armazenar a mensagem
    for (auto &number : numbers) {						//transforma o vetor de inteiros da tabela ascii em caracteres
        chars.push_back(number);						//armazena os caracteres em "chars"
    }
    return chars;										//retorna o vetor de carateres "chars"
}