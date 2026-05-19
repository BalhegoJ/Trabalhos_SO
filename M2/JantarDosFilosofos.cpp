#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <sstream>
#include <pthread.h>
#include <unistd.h>  //Usado para o comando de delay (usleep)

using namespace std;

//Estados possíveis
enum Estado { PENS, FOME, COME };
const string nomes_estados[] = {"PENS", "FOME", "COME"};

//Número de filósofos
int N; 

vector<Estado> estado;
vector<int> refeicoes;
vector<pthread_cond_t> cv_filosofos;
pthread_mutex_t mtx_mesa = PTHREAD_MUTEX_INITIALIZER;
bool simulacao_rodando = true;

// Estrutura para passar múltiplos argumentos para a thread
struct ArgsFilosofo {
    int id;
    int t_p_min, t_p_max, t_c_min, t_c_max;
};

//Funções
string obter_tempo();
void imprimir_estado(int id_filosofo, string mudanca);
int esquerda(int i);
int direita(int i);
void testar(int i);
void pegar_garfos(int i);
void devolver_garfos(int i);
void* rotina_filosofo(void* arg); //Assinatura padrão POSIX

//Main
int main() {
    int duracao = 10; //Segundos 
    int t_p_min = 500, t_p_max = 1500, t_c_min = 500, t_c_max = 1000; //Milissegundos
    N = 5;

    cout << "Iniciando Jantar dos Filósofos (" << N << " filósofos, " << duracao << " segundos)...\n\n";

    estado.assign(N, PENS);
    refeicoes.assign(N, 0);
    
    //Inicializa as variáveis de condição POSIX para cada filósofo
    cv_filosofos.resize(N);
    for (int i = 0; i < N; ++i) {
        pthread_cond_init(&cv_filosofos[i], NULL);
    }
    
    vector<pthread_t> threads(N); 
    vector<ArgsFilosofo*> args(N);

    for (int i = 0; i < N; ++i) {
        //Prepara os argumentos e cria a thread nativa
        args[i] = new ArgsFilosofo{i, t_p_min, t_p_max, t_c_min, t_c_max};
        pthread_create(&threads[i], NULL, rotina_filosofo, (void*)args[i]);
    }

    //usleep (que lê microsegundos)
    usleep(duracao * 1000000); 
    
    //Finalizando
    simulacao_rodando = false;
    for (int i = 0; i < N; ++i) pthread_cond_broadcast(&cv_filosofos[i]); //Acorda todos para saírem
    
    for (int i = 0; i < N; ++i) {
        pthread_join(threads[i], NULL); //Aguarda a thread terminar
        delete args[i]; //Limpa a memória da struct
    }

    cout << "--- RESUMO FINAL ---\n";
    for (int i = 0; i < N; ++i) {
        cout << "F" << i << " comeu " << refeicoes[i] << " vezes.\n";
    }
    return 0;
}

//Interage com o relogio do sistema Operacional para pegar a hora exata e os milssegundos
string obter_tempo() {
    auto agora = chrono::system_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(agora.time_since_epoch()) % 1000;
    time_t tempo_c = chrono::system_clock::to_time_t(agora);
    tm bt = *localtime(&tempo_c);
    ostringstream oss;
    oss << "[" << put_time(&bt, "%H:%M:%S") << '.' << setfill('0') << setw(3) << ms.count() << "]";
    return oss.str();
}

//Impressão do relátorio de estados, sempre que alguém sente fome, começa a comer ou volta a pensar
void imprimir_estado(int id_filosofo, string mudanca) {
    cout << obter_tempo() << " F" << id_filosofo << ": " << mudanca << "\n";
    
    cout << "Garfos: ";
    for (int i = 0; i < N; ++i) {
        int esq = i;
        int dir = (i + 1) % N;
        if (estado[esq] == COME || estado[dir] == COME) cout << "[X] ";
        else cout << "[O] ";
    }
    cout << "\nFilósofos: ";
    for (int i = 0; i < N; ++i) {
        cout << "F" << i << ":" << nomes_estados[estado[i]];
        if (i < N - 1) cout << " | ";
    }
    cout << "\nRefeições: ";
    for (int i = 0; i < N; ++i) {
        cout << "F" << i << ":" << refeicoes[i];
        if (i < N - 1) cout << " | ";
    }
    cout << "\n\n";
}

//Função auxiliar para mapear filosofos ao lado o filosofo atual
int esquerda(int i) { return (i + N - 1) % N; }
int direita(int i) { return (i + 1) % N; }

//Se estiver com fome e os filosofos ao lado não estiverem comendo, muda o estado para comendo e envia um sinal para esse filosofo
void testar(int i) {
    if (estado[i] == FOME && estado[esquerda(i)] != COME && estado[direita(i)] != COME) {
        estado[i] = COME;
        imprimir_estado(i, "FOME --> COME");
        pthread_cond_signal(&cv_filosofos[i]); //Envia o sinal padrão POSIX
    }
}

//Tranca a mesa -> Avisa que está com fome -> Ve se pode comer naquele momento -> Se não pegar garfos destranca o Mutex
void pegar_garfos(int i) {
    pthread_mutex_lock(&mtx_mesa); //Tranca o Mutex POSIX
    estado[i] = FOME;
    imprimir_estado(i, "PENS --> FOME");
    testar(i);
    while (estado[i] != COME && simulacao_rodando) {
        pthread_cond_wait(&cv_filosofos[i], &mtx_mesa); //Dorme e destranca o Mutex temporariamente
    }
    pthread_mutex_unlock(&mtx_mesa); //Destranca ao sair
}

//Tranca a mesa -> Avisa que esta pensando e aumenta o numero de refeições -> Avisa aos Filosofos ao lado que terminou a refeição
void devolver_garfos(int i) {
    pthread_mutex_lock(&mtx_mesa); //Tranca o Mutex POSIX
    estado[i] = PENS;
    refeicoes[i]++;
    imprimir_estado(i, "COME --> PENS");
    testar(esquerda(i));
    testar(direita(i));
    pthread_mutex_unlock(&mtx_mesa); //Destranca ao sair
}

void* rotina_filosofo(void* arg) {
    //Resgata os argumentos passados para a thread
    ArgsFilosofo* args = (ArgsFilosofo*) arg;
    int i = args->id;
    
    //Gerador de numeros aleatorio (1 para cada thread)
    mt19937 rng(random_device{}() + i);

    //Sorteia qualquer tempo dentro da faixa definida
    uniform_int_distribution<int> dist_pensar(args->t_p_min, args->t_p_max);
    uniform_int_distribution<int> dist_comer(args->t_c_min, args->t_c_max);

    while (simulacao_rodando) {
        usleep(dist_pensar(rng) * 1000); //usleep requer tempo em microsegundos (ms * 1000)
        if (!simulacao_rodando) break;
        
        pegar_garfos(i);
        
        if (!simulacao_rodando) break;
        
        usleep(dist_comer(rng) * 1000); //usleep requer tempo em microsegundos (ms * 1000)
        
        devolver_garfos(i);
    }
    
    return NULL; //Threads POSIX devem retornar NULL ao finalizar
}