#include "intercalacao.h"
#include "funcionarios.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct no
{
    int codFuncionario;
    //Indice onde o funcionário encontra-se na pilha
    int indiceParticao;
    struct no *esq;
    struct no *dir;
} TNo;

void removerPrimeiroItem(TNo *vetNo[], int *num_p);
void adicionarFimLista(TNo *vetNo[], TNo *tempNo, int num_p);
void gravarFuncionario(TPilha **pilha, int *vetTop, FILE *arquivoSaida, int *posArqSaida, int indiceParticao);


void intercalacao_basico(char *nome_arquivo_saida, int num_p, Lista *nome_particoes)
{

    int fim = 0; //variavel que controla fim do procedimento
    FILE *out; //declara ponteiro para arquivo

    //abre arquivo de saida para escrita
    if ((out = fopen(nome_arquivo_saida, "wb")) == NULL)
    {
        printf("Erro ao abrir arquivo de sa?da\n");
    }
    else
    {
        //cria vetor de particoes
        TVet v[num_p];

        //abre arquivos das particoes, colocando variavel de arquivo no campo f do vetor
        //e primeiro funcionario do arquivo no campo func do vetor
        for (int i=0; i < num_p; i++)
        {
            v[i].f = fopen(nome_particoes->nome, "rb");
            v[i].aux_p = 0;
            if (v[i].f != NULL)
            {
                fseek(v[i].f, v[i].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[i].f);
                if (f == NULL)
                {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posi??o do vetor
                    v[i].func = funcionario(INT_MAX, "","","",0);
                }
                else
                {
                    //conseguiu ler funcionario, coloca na posi??o atual do vetor
                    v[i].func = f;
                }
            }
            else
            {
                fim = 1;
            }
            nome_particoes = nome_particoes->prox;
        }

        int aux = 0;
        while (!(fim))   //conseguiu abrir todos os arquivos
        {
            int menor = INT_MAX;
            int pos_menor;
            //encontra o funcionario com menor chave no vetor
            for(int i = 0; i < num_p; i++)
            {
                if(v[i].func->cod < menor)
                {
                    menor = v[i].func->cod;
                    pos_menor = i;
                }
            }
            if (menor == INT_MAX)
            {
                fim = 1; //terminou processamento
            }
            else
            {
                //salva funcionario no arquivo de saída
                fseek(out, aux * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[pos_menor].func, out);
                //atualiza posição pos_menor do vetor com pr?ximo funcionario do arquivo
                v[pos_menor].aux_p++;
                fseek(v[pos_menor].f, v[pos_menor].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[pos_menor].f);
                aux++;
                if (f == NULL)
                {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posiçao do vetor
                    v[pos_menor].func = funcionario(INT_MAX, "", "", "",0.0);
                }
                else
                {
                    v[pos_menor].func = f;
                }

            }
        }

        //fecha arquivos das partiÇões de entrada
        for(int i = 0; i < num_p; i++)
        {
            fclose(v[i].f);
            //    free(v[i].func);
        }
        //fecha arquivo de saída
        fclose(out);
    }
}

void intercalacao_arvore_de_vencedores(TPilha **pilha, int *vetTop, char *nome_arquivo_saida, int num_p)
{
    TNo *vetNo[num_p];
    FILE *arquivoSaida = fopen(nome_arquivo_saida, "wb+");
    rewind(arquivoSaida);
    int posArqSaida = 0;

    do
    {

        int indiceVetorNo = 0;

        printf("\n\n");
        //Populando a lista de nós para fazer a classificação
        for (int i = 0; i < num_p; i++)
        {
            //Verificando se pilha tem item
            if (vetTop[i] >= 0)
            {

                TNo *n = (TNo *)malloc(sizeof(TNo));

                //peek_func é a função responsável em retornar o funcionário
                TFunc *funcionario = peek_func(pilha[i], 0, &vetTop[i]);

                printf("pilha[%d] =======> %d \n", i, funcionario->cod);

                n->codFuncionario = funcionario->cod;
                n->indiceParticao = i;

                vetNo[indiceVetorNo] = n;
                indiceVetorNo++;
            }
        }

        //Se possuir somente um item no vetor de No
        //É pq a classificação ja chegou ao fim e necessita somente de gravar
        //o item no arquivo de saida
        if (vetNo[1] == NULL)
        {
            gravarFuncionario(pilha, vetTop, arquivoSaida, posArqSaida, vetNo[0]->indiceParticao);
            break;
        }

        //Classificação do vencedor entre os dois primeiros nós da lista
        do
        {
            printf("\n\n");
            for (int i = 0; i < num_p; i++)
            {
                if (vetNo[i] != NULL)
                    printf("vetNo[%d] =======> %d \n", i, vetNo[i]->codFuncionario);
            }

            //Nós de comparação
            TNo *noEsquerda = (TNo *)malloc(sizeof(TNo));
            TNo *noDireita = (TNo *)malloc(sizeof(TNo));

            //Novo nó que será adicionado ao fim da lista
            //após a definição do vencedor, no qual será alocado
            //na raiz deste nó
            TNo *n = (TNo *)malloc(sizeof(TNo));

            noEsquerda = vetNo[0];
            removerPrimeiroItem(vetNo, num_p);

            noDireita = vetNo[0];
            removerPrimeiroItem(vetNo, num_p);

            //Definindo vencedor entre os dois nós
            n->codFuncionario = noEsquerda->codFuncionario > noDireita->codFuncionario ? noDireita->codFuncionario : noEsquerda->codFuncionario;
            n->indiceParticao = noEsquerda->codFuncionario > noDireita->codFuncionario ? noDireita->indiceParticao : noEsquerda->indiceParticao;
            n->esq = noEsquerda;
            n->dir = noDireita;

            adicionarFimLista(vetNo, n, num_p);

        } while (vetNo[1] != NULL);

        //Ao achar o vencedor entre todos os números
        //Ele é gravado no arquivo de saída e retirado da pilha
        //Para que o algorítmo reinicie e ache o próximo
        //vencedor entre os restantes na pilha
        gravarFuncionario(pilha, vetTop, arquivoSaida, posArqSaida, vetNo[0]->indiceParticao);
        posArqSaida++;

    } while (1 == 1);

    fclose(arquivoSaida);

    printf("\n\n**************************** RESULTADO INTERCALACAO ****************************\n\n");
}

void removerPrimeiroItem(TNo *vetNo[], int *num_p)
{
    for (int i = 0; i < num_p; i++)
    {
        if (i + 1 < num_p && vetNo[i + 1] != NULL)
        {
            vetNo[i] = vetNo[i + 1];
        }
        else
        {
            vetNo[i] = NULL;
        }
    }
}


void adicionarFimLista(TNo *vetNo[], TNo *tempNo, int num_p)
{
    //Método que adiciona o nó no fim da lista
    for (int i = 0; i < num_p; i++)
    {
        if (vetNo[i] == NULL)
        {
            vetNo[i] = tempNo;
            break;
        }
    }
}

void gravarFuncionario(TPilha **pilha, int *vetTop, FILE *arquivoSaida, int *posArqSaida, int indiceParticao)
{
    TFunc *funcionario = peek_func(pilha[indiceParticao], 0, &vetTop[indiceParticao]);

    //Gravar funcionario no arquivo
    salva_ArqSaida(arquivoSaida, posArqSaida, funcionario);

    //Removendo item gravado da pilha
    pop(pilha[indiceParticao], 0, &vetTop[indiceParticao]);
}

