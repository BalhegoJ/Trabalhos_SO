#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <cctype>
#include <algorithm>

using namespace std;

// 1. ESTRUTURAS DE DADOS (MAPEAMENTO DA FAT16)

#pragma pack(push, 1) //Garante que a struct tenha exatamente o tamanho dos bytes
struct BootBlock {
    uint8_t  jmp[3];
    uint8_t  oem[8];
    uint16_t bytes_per_block;
    uint8_t  blocks_per_cluster;
    uint16_t reserved_blocks;
    uint8_t  num_fats;
    uint16_t root_entries;
    uint16_t total_blocks_16;
    uint8_t  media_descriptor;
    uint16_t blocks_per_fat;
};

struct DirEntry {
    uint8_t  filename[8];      //0x00
    uint8_t  extension[3];     //0x08
    uint8_t  attributes;       //0x0b
    uint8_t  reserved[10];     //0x0c
    uint16_t time;             //0x16
    uint16_t date;             //0x18
    uint16_t starting_cluster; //0x1a
    uint32_t file_size;        //0x1c
};
#pragma pack(pop)

//2. FUNÇÕES AUXILIARES

//Formata uma string "nome.txt" para o padrão FAT de 11 bytes: "NOME    TXT"
void formatar_nome_fat(const string& nome_original, uint8_t* nome_fat_11) {
    memset(nome_fat_11, ' ', 11);
    size_t i = 0, j = 0;
    
    //Extrai o nome
    while (i < nome_original.length() && nome_original[i] != '.' && j < 8) {
        nome_fat_11[j++] = toupper(nome_original[i++]);
    }
    //Avança para a extensão
    while (i < nome_original.length() && nome_original[i] != '.') i++;
    
    if (i < nome_original.length() && nome_original[i] == '.') {
        i++;
        j = 8;
        //Extrai a extensão
        while (i < nome_original.length() && j < 11) {
            nome_fat_11[j++] = toupper(nome_original[i++]);
        }
    }
}

//Extrai e imprime data e hora dos bits comprimidos
void exibir_data_hora_fat(uint16_t date, uint16_t time) {
    int dia = date & 0x1F;
    int mes = (date >> 5) & 0x0F;
    int ano = 1980 + ((date >> 9) & 0x7F);
    
    int segundos = (time & 0x1F) * 2;
    int minutos = (time >> 5) & 0x3F;
    int horas = (time >> 11) & 0x1F;
    
    cout << "  Modificado em: " 
         << setfill('0') << setw(2) << dia << "/" 
         << setw(2) << mes << "/" 
         << setw(4) << ano << " as "
         << setw(2) << horas << ":" 
         << setw(2) << minutos << ":" 
         << setw(2) << segundos << "\n";
}

//Busca um arquivo no Root Directory
bool encontrar_arquivo(fstream& disk, const BootBlock& boot, const string& nome_busca, DirEntry& out_entry, uint32_t& out_offset) {
    uint32_t root_dir_offset = (boot.reserved_blocks + (boot.blocks_per_fat * boot.num_fats)) * boot.bytes_per_block;
    uint8_t nome_formatado[11];
    formatar_nome_fat(nome_busca, nome_formatado);

    for (int i = 0; i < boot.root_entries; i++) {
        uint32_t current_offset = root_dir_offset + (i * 32);
        disk.seekg(current_offset);
        disk.read(reinterpret_cast<char*>(&out_entry), sizeof(DirEntry));

        if (out_entry.filename[0] == 0x00) break; // Fim da lista
        if (out_entry.filename[0] == 0xE5) continue; // Arquivo apagado

        if (memcmp(out_entry.filename, nome_formatado, 11) == 0) {
            out_offset = current_offset;
            return true;
        }
    }
    return false;
}

//3. OPERAÇÕES PRINCIPAIS DO TRABALHO
void listar_diretorio_raiz(fstream& disk, const BootBlock& boot) {
    uint32_t root_dir_offset = (boot.reserved_blocks + (boot.blocks_per_fat * boot.num_fats)) * boot.bytes_per_block;
    disk.seekg(root_dir_offset);
    DirEntry entry;

    cout << "\n--- ARQUIVOS NO DIRETORIO RAIZ ---\n";
    for (int i = 0; i < boot.root_entries; i++) {
        disk.read(reinterpret_cast<char*>(&entry), sizeof(DirEntry));

        if (entry.filename[0] == 0x00) break;
        if (entry.filename[0] == 0xE5) continue; 

        if ((entry.attributes & 0x08) == 0) { //Ignora Volume Label
            cout << "\nNome: ";
            for(int j=0; j<8 && entry.filename[j]!=' '; ++j) cout << entry.filename[j];
            if(entry.extension[0] != ' ') {
                cout << ".";
                for(int j=0; j<3 && entry.extension[j]!=' '; ++j) cout << entry.extension[j];
            }
            
            cout << " | Tamanho: " << entry.file_size << " bytes\n";
            exibir_data_hora_fat(entry.date, entry.time);
            
            if (entry.attributes & 0x01) cout << "  [Somente Leitura]\n";
            if (entry.attributes & 0x02) cout << "  [Oculto]\n";
            if (entry.attributes & 0x04) cout << "  [Sistema]\n";
        }
    }
}

void ler_arquivo(fstream& disk, const BootBlock& boot, uint16_t start_cluster, uint32_t file_size) {
    uint32_t fat_offset = boot.reserved_blocks * boot.bytes_per_block;
    uint32_t root_dir_size = boot.root_entries * 32;
    uint32_t data_offset = fat_offset + (boot.blocks_per_fat * boot.num_fats * boot.bytes_per_block) + root_dir_size;
    uint32_t bytes_per_cluster = boot.blocks_per_cluster * boot.bytes_per_block;

    uint16_t current_cluster = start_cluster;
    uint32_t bytes_read = 0;
    vector<uint8_t> buffer(bytes_per_cluster);

    cout << "\n--- CONTEUDO DO ARQUIVO ---\n";
    while (current_cluster >= 0x0002 && current_cluster <= 0xFFEF && bytes_read < file_size) {
        uint32_t cluster_location = data_offset + ((current_cluster - 2) * bytes_per_cluster);
        
        disk.seekg(cluster_location);
        disk.read(reinterpret_cast<char*>(buffer.data()), bytes_per_cluster);

        uint32_t bytes_to_print = (file_size - bytes_read) < bytes_per_cluster ? (file_size - bytes_read) : bytes_per_cluster;
        for (uint32_t i = 0; i < bytes_to_print; i++) {
            cout << static_cast<char>(buffer[i]);
        }
        bytes_read += bytes_to_print;

        disk.seekg(fat_offset + (current_cluster * 2));
        disk.read(reinterpret_cast<char*>(&current_cluster), sizeof(uint16_t));
    }
    cout << "\n";
}

void renomear_arquivo(fstream& disk, const BootBlock& boot, const string& nome_antigo, const string& nome_novo) {
    DirEntry entry;
    uint32_t entry_offset;
    
    if (encontrar_arquivo(disk, boot, nome_antigo, entry, entry_offset)) {
        uint8_t formatado_novo[11];
        formatar_nome_fat(nome_novo, formatado_novo);
        
        disk.seekp(entry_offset);
        disk.write(reinterpret_cast<char*>(formatado_novo), 11);
        cout << "Sucesso! Arquivo renomeado para '" << nome_novo << "'.\n";
    } else {
        cout << "Erro: Arquivo '" << nome_antigo << "' nao encontrado.\n";
    }
}

void apagar_arquivo(fstream& disk, const BootBlock& boot, const string& nome) {
    DirEntry entry;
    uint32_t entry_offset;
    
    if (encontrar_arquivo(disk, boot, nome, entry, entry_offset)) {
        uint8_t deleted_marker = 0xE5;
        disk.seekp(entry_offset);
        disk.write(reinterpret_cast<char*>(&deleted_marker), sizeof(uint8_t));

        uint32_t fat_offset = boot.reserved_blocks * boot.bytes_per_block;
        uint16_t current_cluster = entry.starting_cluster;
        uint16_t free_cluster = 0x0000;

        while (current_cluster >= 0x0002 && current_cluster <= 0xFFEF) {
            uint16_t next_cluster;
            disk.seekg(fat_offset + (current_cluster * 2));
            disk.read(reinterpret_cast<char*>(&next_cluster), sizeof(uint16_t));

            disk.seekp(fat_offset + (current_cluster * 2));
            disk.write(reinterpret_cast<char*>(&free_cluster), sizeof(uint16_t));

            current_cluster = next_cluster;
        }
        cout << "Sucesso! Arquivo '" << nome << "' foi apagado.\n";
    } else {
        cout << "Erro: Arquivo '" << nome << "' nao encontrado.\n";
    }
}

void inserir_arquivo(fstream& disk, const BootBlock& boot, const string& caminho_local, const string& nome_destino) {
    ifstream local_file(caminho_local, ios::binary | ios::ate);
    if (!local_file) {
        cout << "Erro: Arquivo '" << caminho_local << "' nao encontrado no sistema local.\n";
        return;
    }

    uint32_t file_size = local_file.tellg();
    local_file.seekg(0, ios::beg);

    uint32_t fat_offset = boot.reserved_blocks * boot.bytes_per_block;
    uint32_t root_dir_offset = fat_offset + (boot.blocks_per_fat * boot.num_fats * boot.bytes_per_block);
    uint32_t data_offset = root_dir_offset + (boot.root_entries * 32);
    uint32_t bytes_per_cluster = boot.blocks_per_cluster * boot.bytes_per_block;

    DirEntry entry;
    uint32_t entry_offset = 0;
    bool entry_found = false;

    for (int i = 0; i < boot.root_entries; i++) {
        disk.seekg(root_dir_offset + (i * 32));
        disk.read(reinterpret_cast<char*>(&entry), sizeof(DirEntry));
        if (entry.filename[0] == 0x00 || entry.filename[0] == 0xE5) {
            entry_offset = root_dir_offset + (i * 32);
            entry_found = true;
            break;
        }
    }

    if (!entry_found) {
        cout << "Erro: O Diretorio Raiz esta cheio.\n";
        return;
    }

    uint16_t first_cluster = 0;
    uint16_t prev_cluster = 0;
    uint32_t bytes_left = file_size;
    vector<uint8_t> buffer(bytes_per_cluster, 0);
    uint32_t total_clusters = (boot.blocks_per_fat * boot.bytes_per_block) / 2;

    while (bytes_left > 0) {
        uint16_t free_cluster = 0;
        uint16_t fat_value;
        
        for (uint16_t c = 2; c < total_clusters; c++) {
            disk.seekg(fat_offset + (c * 2));
            disk.read(reinterpret_cast<char*>(&fat_value), sizeof(uint16_t));
            if (fat_value == 0x0000) {
                free_cluster = c;
                break;
            }
        }

        if (free_cluster == 0) {
            cout << "Erro: Disco cheio, sem clusters livres.\n";
            return;
        }

        if (first_cluster == 0) first_cluster = free_cluster;
        if (prev_cluster != 0) {
            disk.seekp(fat_offset + (prev_cluster * 2));
            disk.write(reinterpret_cast<char*>(&free_cluster), sizeof(uint16_t));
        }

        uint16_t eof_marker = 0xFFFF;
        disk.seekp(fat_offset + (free_cluster * 2));
        disk.write(reinterpret_cast<char*>(&eof_marker), sizeof(uint16_t));

        uint32_t chunk_size = (bytes_left < bytes_per_cluster) ? bytes_left : bytes_per_cluster;
        fill(buffer.begin(), buffer.end(), 0); 
        local_file.read(reinterpret_cast<char*>(buffer.data()), chunk_size);

        uint32_t cluster_location = data_offset + ((free_cluster - 2) * bytes_per_cluster);
        disk.seekp(cluster_location);
        disk.write(reinterpret_cast<char*>(buffer.data()), bytes_per_cluster);

        bytes_left -= chunk_size;
        prev_cluster = free_cluster;
    }

    memset(&entry, 0, sizeof(DirEntry));
    uint8_t nome_formatado[11];
    formatar_nome_fat(nome_destino, nome_formatado);
    memcpy(entry.filename, nome_formatado, 11);
    
    entry.attributes = 0x20; 
    entry.starting_cluster = first_cluster;
    entry.file_size = file_size;            
    entry.time = 0x0000; 
    entry.date = 0x0021; 
    
    disk.seekp(entry_offset);
    disk.write(reinterpret_cast<char*>(&entry), sizeof(DirEntry));

    cout << "Sucesso! Arquivo inserido na imagem como '" << nome_destino << "'.\n";
}

// 4. FUNÇÃO PRINCIPAL (MENU)

int main() {
    fstream disk("disco.img", ios::in | ios::out | ios::binary);
    if (!disk) {
        cout << "Erro ao abrir a imagem do disco. Verifique se o arquivo 'disco.img' existe.\n";
        return 1;
    }

    BootBlock boot;
    disk.seekg(0);
    disk.read(reinterpret_cast<char*>(&boot), sizeof(BootBlock));

    int opcao = -1;
    string string1, string2;
    DirEntry entry;
    uint32_t dummy_offset;

    while (opcao != 0) {
        cout << "\n========== MENU FAT16 ==========\n"
             << "1 - Listar arquivos e atributos\n"
             << "2 - Ler conteudo de um arquivo\n"
             << "3 - Renomear um arquivo\n"
             << "4 - Inserir novo arquivo\n"
             << "5 - Apagar um arquivo\n"
             << "0 - Sair\n"
             << "Escolha uma opcao: ";
        
        if (!(cin >> opcao)) { 
            cin.clear(); 
            cin.ignore(10000, '\n'); 
            continue;
        }

        switch (opcao) {
            case 1:
                listar_diretorio_raiz(disk, boot);
                break;
            case 2:
                cout << "Digite o nome do arquivo (ex: Arquivo.txt): ";
                cin >> string1;
                if (encontrar_arquivo(disk, boot, string1, entry, dummy_offset)) {
                    ler_arquivo(disk, boot, entry.starting_cluster, entry.file_size);
                } else {
                    cout << "Erro: Arquivo '" << string1 << "' nao encontrado.\n";
                }
                break;
            case 3:
                cout << "Digite o nome atual do arquivo (ex: old.txt): ";
                cin >> string1;
                cout << "Digite o novo nome (ex: new.txt): ";
                cin >> string2;
                renomear_arquivo(disk, boot, string1, string2);
                break;
            case 4:
                cout << "Digite o caminho/nome do arquivo local (na VM): ";
                cin >> string1;
                cout << "Digite o nome final dele na imagem FAT16 (ex: arq.txt): ";
                cin >> string2;
                inserir_arquivo(disk, boot, string1, string2);
                break;
            case 5:
                cout << "Digite o nome do arquivo a ser apagado (ex: lixo.txt): ";
                cin >> string1;
                apagar_arquivo(disk, boot, string1);
                break;
            case 0:
                cout << "Encerrando...\n";
                break;
            default:
                cout << "Opcao invalida!\n";
        }
    }

    disk.close();
    return 0;
}