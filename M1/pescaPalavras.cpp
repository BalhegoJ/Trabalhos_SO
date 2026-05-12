#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cctype>


using namespace std;

// Estrutura para armazenar o resultado da busca de cada palavra
struct ResultadoPalavra {
    string palavra;
    bool encontrada;
    int linha_inicial;
    int coluna_inicial;
    string direcao;
    vector<pair<int, int>> caminho; // Coordenadas de cada letra para transformar em maiúscula
};

//Vetores de direção: Horizontal, Vertical e Diagonais
//Ordem: direita, esquerda, baixo, cima, dir/baixo, esq/baixo, dir/cima, esq/cima
const int delta_linha[] = {0, 0, 1, -1, 1, 1, -1, -1};
const int delta_coluna[] = {1, -1, 0, 0, 1, -1, 1, -1};
const string nomes_direcoes[] = {
    "direita", "esquerda", "baixo", "cima", 
    "direita/baixo", "esquerda/baixo", "direita/cima", "esquerda/cima"
};

//Variáveis globais para armazenar os resultados e o mutex para sincronização
vector<ResultadoPalavra> resultados_globais;
mutex mutex_resultados;

//Função para converter string para minúscula
string paraMinuscula(string texto) {
    transform(texto.begin(), texto.end(), texto.begin(), [](unsigned char c){ return tolower(c); });
    return texto;
}

//Função executada por cada Thread: Busca apenas UMA palavra
void buscarPalavraUnica(const vector<string>& matriz, const string& palavra_alvo) {
    int total_linhas = matriz.size();
    int total_colunas = matriz[0].size();
    string alvo_minusculo = paraMinuscula(palavra_alvo);

    ResultadoPalavra resultado;
    resultado.palavra = palavra_alvo; //Guarda a palavra original para a saída
    resultado.encontrada = false;

    //Varredura da matriz
    for (int linha = 0; linha < total_linhas && !resultado.encontrada; ++linha) {
        for (int coluna = 0; coluna < total_colunas && !resultado.encontrada; ++coluna) {
            
            //Se a primeira letra bater, checamos todas as direções
            if (tolower(matriz[linha][coluna]) == alvo_minusculo[0]) {
                for (int direcao = 0; direcao < 8; ++direcao) {
                    int linha_atual = linha;
                    int coluna_atual = coluna;
                    bool combina = true;
                    vector<pair<int, int>> caminho_atual;

                    //Verificação das letras da palavra
                    for (unsigned k = 0; k < alvo_minusculo.length(); ++k) {
                        //Verifica limites da matriz
                        if (linha_atual < 0 || linha_atual >= total_linhas || coluna_atual < 0 || coluna_atual >= total_colunas) {
                            combina = false;
                            break;
                        }
                        //Verifica se as letras coincidem
                        if (tolower(matriz[linha_atual][coluna_atual]) != alvo_minusculo[k]) {
                            combina = false;
                            break;
                        }
                        
                        caminho_atual.push_back({linha_atual, coluna_atual});
                        linha_atual += delta_linha[direcao];
                        coluna_atual += delta_coluna[direcao];
                    }

                    //Se encontrou a palavra completa
                    if (combina) {
                        resultado.encontrada = true;
                        //+1 pois o formato de saída exige índice base 1 (ex: 1,26)
                        resultado.linha_inicial = linha + 1; 
                        resultado.coluna_inicial = coluna + 1;
                        resultado.direcao = nomes_direcoes[direcao];
                        resultado.caminho = caminho_atual;
                        break; //Sai do loop de direções
                    }
                }
            }
        }
    }

    //Salvando o resultado no vetor global
    //O lock_guard garante que a thread libere o mutex automaticamente ao sair deste escopo
    lock_guard<mutex> trava(mutex_resultados);
    resultados_globais.push_back(resultado);
}

int main() {

    ifstream arquivo("cacapalavras.txt");

    //1. Tratamento de Erros Básicos
    if (!arquivo.is_open()) {
        cerr << "Erro: Nao foi possivel abrir o arquivo cacapalavras.txt" << endl;
        cerr << "Verifique se o arquivo existe no mesmo diretorio do executavel." << endl;
        return 1;
    }

    int linhas, colunas;
    if (!(arquivo >> linhas >> colunas)) {
        cerr << "Erro: Formato de dimensoes invalido no arquivo." << endl;
        return 1;
    }

    //2. Leitura da Matriz
    vector<string> matriz(linhas);
    for (int i = 0; i < linhas; ++i) {
        arquivo >> matriz[i];
    }

    //3. Leitura da Lista de Palavras
    vector<string> palavras;
    string palavra_lida;
    while (arquivo >> palavra_lida) {
        palavras.push_back(palavra_lida);
    }
    arquivo.close();

    //4. Configuração das Threads (Uma thread por palavra)
    vector<thread> threads_busca;
    
    //Lança uma thread para cada palavra lida
    for (unsigned i = 0; i < palavras.size(); ++i) {
        threads_busca.emplace_back(buscarPalavraUnica, ref(matriz), palavras[i]);
    }

    //Sincronização: Main thread aguarda TODAS as threads filhas finalizarem 
    for (unsigned i = 0; i < threads_busca.size(); ++i) {
        if (threads_busca[i].joinable()) {
            threads_busca[i].join();
        }
    }

    //5. Destacando palavras encontradas (Transformando em Maiúsculas)
    //Feito sequencialmente após a busca para evitar inconsistências e condições de corrida
    for (unsigned i = 0; i < resultados_globais.size(); ++i) {
        if (resultados_globais[i].encontrada) {
            //Percorre o caminho da palavra encontrada com for convencional
            for (size_t j = 0; j < resultados_globais[i].caminho.size(); ++j) {
                int linha_coord = resultados_globais[i].caminho[j].first;
                int coluna_coord = resultados_globais[i].caminho[j].second;
                matriz[linha_coord][coluna_coord] = toupper(matriz[linha_coord][coluna_coord]);
            }
        }
    }

    //6. Impressão da Matriz Final
    for (unsigned i = 0; i < matriz.size(); ++i) {
        cout << matriz[i] << "\n";
    }

    //7. Impressão do Relatório de Palavras
    cout << "\n--- Relatorio de Buscas ---\n";
    //Ordenar os resultados na mesma ordem do arquivo de entrada para ficar legível
    for (unsigned i = 0; i < palavras.size(); ++i) {
        for (unsigned j = 0; j < resultados_globais.size(); ++j) {
            if (resultados_globais[j].palavra == palavras[i]) {
                if (resultados_globais[j].encontrada) {
                    cout << resultados_globais[j].palavra << " (" 
                         << resultados_globais[j].linha_inicial << "," 
                         << resultados_globais[j].coluna_inicial << "): " 
                         << resultados_globais[j].direcao << "\n";
                } else {
                    cout << resultados_globais[j].palavra << ": nao encontrada\n";
                }
                break;
            }
        }
    }

    return 0;
}