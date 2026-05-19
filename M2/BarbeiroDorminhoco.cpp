#include <iostream>
#include <queue>
#include <chrono>
#include <random>
#include <iomanip>
#include <sstream>
#include <pthread.h>
#include <unistd.h>  //Usado para o comando de delay (usleep)

using namespace std;


int N_cadeiras = 5;

//Primeiro a chegar é o primeiro a sair(FIFO)
queue<int> fila_espera;
int cont_atendidos = 0;
int cont_desistentes = 0;
int em_espera = 0;

enum EstadoBarbeiro { DORME, ATENDE };
EstadoBarbeiro estado_barbeiro = DORME;
int cliente_sendo_atendido = -1; //-1 significa nenhum

//"Porta da barbearia", uma pessoa(thread) por vez para checar a fila, sentar ou ir embora
pthread_mutex_t mtx_barbearia = PTHREAD_MUTEX_INITIALIZER; //Mutex POSIX

//Obarbeiro vê que a fila está vazia, ele vai dormir e só acorda quando rcebe um acionamento
pthread_cond_t cv_barbeiro = PTHREAD_COND_INITIALIZER; //Variável de Condição POSIX
bool barbearia_aberta = true;

// Estrutura para passar os tempos para a rotina do barbeiro
struct ArgsBarbeiro {
    int t_atend_min;
    int t_atend_max;
};

//Funções
string obter_tempo();
void imprimir_estado(string evento);
void* rotina_barbeiro(void* arg);
void* chegada_cliente(void* arg);

//Main
int main() {
    int duracao_simulacao = 15;
    int t_chegada_min = 300, t_chegada_max = 1200;
    int t_atend_min = 800, t_atend_max = 2000;
    
    cout << "Iniciando Barbearia (" << N_cadeiras << " cadeiras, " << duracao_simulacao << " segundos)...\n\n";

    //Prepara argumentos e cria a thread do barbeiro no padrão POSIX
    ArgsBarbeiro* args_barbeiro = new ArgsBarbeiro{t_atend_min, t_atend_max};
    pthread_t thread_barbeiro;
    pthread_create(&thread_barbeiro, NULL, rotina_barbeiro, (void*)args_barbeiro);

    mt19937 rng(random_device{}());
    uniform_int_distribution<int> dist_chegada(t_chegada_min, t_chegada_max);

    auto inicio = chrono::system_clock::now();
    int id_cliente = 1;

    //Loop principal gera clientes
    while (chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - inicio).count() < duracao_simulacao) {
        usleep(dist_chegada(rng) * 1000); //Usleep requer microsegundos
        
        //Cria a thread do cliente e passa o ID de forma segura alocando na memória
        pthread_t t_cliente;
        int* id_ptr = new int(id_cliente++); 
        pthread_create(&t_cliente, NULL, chegada_cliente, (void*)id_ptr);
        
        //A thread do cliente executa a chegada e morre em seguida (Substitui o t_cliente.detach())
        pthread_detach(t_cliente); 
    }

    //Encerrando
    pthread_mutex_lock(&mtx_barbearia); //Trava explicita POSIX
    barbearia_aberta = false;
    imprimir_estado("A barbearia fechou as portas! (Não entram mais clientes)");
    pthread_mutex_unlock(&mtx_barbearia); //Destrava explicita POSIX

    pthread_cond_broadcast(&cv_barbeiro); //Acorda o barbeiro (substitui notify_all)
    pthread_join(thread_barbeiro, NULL);  //Aguarda o barbeiro terminar
    
    delete args_barbeiro; //Limpa memória

    cout << "--- RESUMO FINAL ---\n"
         << "Total de clientes atendidos: " << cont_atendidos << "\n"
         << "Total de clientes que desistiram: " << cont_desistentes << "\n";

    return 0;
}

//Obtem o tempo do sistema operacional e formata em [HH:MM:SS.mmm]
string obter_tempo() {
    auto agora = chrono::system_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(agora.time_since_epoch()) % 1000;
    time_t tempo_c = chrono::system_clock::to_time_t(agora);
    tm bt = *localtime(&tempo_c);
    ostringstream oss;
    oss << "[" << put_time(&bt, "%H:%M:%S") << '.' << setfill('0') << setw(3) << ms.count() << "]";
    return oss.str();
}

//Diz se o barbeiro está atendendo ou dormindo, mostra a fila e a contagem de atendimentos
void imprimir_estado(string evento) {
    cout << obter_tempo() << " " << evento << "\n";
    if (estado_barbeiro == DORME) {
        cout << "Barbeiro: DORME\n";
    } else {
        cout << "Barbeiro: ATENDE C" << cliente_sendo_atendido << "\n";
    }
    
    cout << "Fila: [";
    for(int i = 0; i < N_cadeiras; i++) {
        if(i < em_espera) cout << "°";
        else cout << ".";
    }
    cout << "] (" << em_espera << "/" << N_cadeiras << ")";
    
    if (em_espera > 0) {
        cout << " -> ";
        queue<int> fila_temp = fila_espera;
        while (!fila_temp.empty()) {
            cout << "C" << fila_temp.front() << " ";
            fila_temp.pop();
        }
    }
    
    cout << "\nContadores: atendidos = " << cont_atendidos 
         << " | desistentes = " << cont_desistentes 
         << " | em espera = " << em_espera << "\n\n";
}

//Usamos um gerador de numeros aleatorios e uma distribuição.
//Barbeiro trabalha enquanto tiver gente sentada e enquanto estiver aberta a barbearia
//Se fila vazia, barbeiro dorme, só acorda se chegar cliente na fila ou acabar o expediente
void* rotina_barbeiro(void* arg) {
    ArgsBarbeiro* args = (ArgsBarbeiro*) arg;
    mt19937 rng(random_device{}());
    uniform_int_distribution<int> dist_atend(args->t_atend_min, args->t_atend_max);

    while (barbearia_aberta || em_espera > 0) {
        pthread_mutex_lock(&mtx_barbearia); //Tranca a barbearia POSIX
        
        while (fila_espera.empty() && barbearia_aberta) {
            if (estado_barbeiro != DORME) {
                estado_barbeiro = DORME;
                imprimir_estado("Barbeiro dormiu pois não há clientes");
            }
            pthread_cond_wait(&cv_barbeiro, &mtx_barbearia); //Dorme e solta o mutex
        }
        
        //Chama o primeiro da fila tirando ele da cadeira de espera
        if (!fila_espera.empty()) {
            cliente_sendo_atendido = fila_espera.front();
            fila_espera.pop();
            em_espera--;
            estado_barbeiro = ATENDE;
            imprimir_estado("Barbeiro iniciou atendimento do cliente C" + to_string(cliente_sendo_atendido));
            
            //Destranca a barbearia e se ocupa no corte de cabelo, clientes novos podem chegar e olhar quantas cadeira tem e sentar
            pthread_mutex_unlock(&mtx_barbearia);

            usleep(dist_atend(rng) * 1000); //usleep requer microsegundos
            
            //Quando o corte termina, tranca o sistema e atualiza os contadores
            pthread_mutex_lock(&mtx_barbearia);
            
            cont_atendidos++;
            imprimir_estado("Barbeiro concluiu atendimento do cliente C" + to_string(cliente_sendo_atendido));
            cliente_sendo_atendido = -1;
        }
        
        pthread_mutex_unlock(&mtx_barbearia); //Destranca a barbearia ao fim do ciclo
    }
    return NULL;
}


//Cliente chega e tranca a porta, Verifica se a sala está cheia, caso entre incrementa clientes em espera e aciona o barbeiro para caso ele esteja dormindo (notify.one)
void* chegada_cliente(void* arg) {
    //Resgata o ID e limpa a memória alocada
    int id_cliente = *((int*)arg);
    delete (int*)arg;

    pthread_mutex_lock(&mtx_barbearia); //Tranca a porta POSIX
    
    if (em_espera == N_cadeiras) {
        cont_desistentes++;
        imprimir_estado("Cliente C" + to_string(id_cliente) + " chegou, mas desistiu por falta de cadeira");
    } else {
        fila_espera.push(id_cliente);
        em_espera++;
        imprimir_estado("Cliente C" + to_string(id_cliente) + " chegou e entrou na fila");
        pthread_cond_signal(&cv_barbeiro); //Acorda o barbeiro (substitui notify_one)
    }

    pthread_mutex_unlock(&mtx_barbearia); //Destranca a porta POSIX
    return NULL;
}