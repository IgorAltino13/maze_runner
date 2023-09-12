#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <stack>
#include <unistd.h>
#include <stdio.h>

// Matriz de char representando o labirinto
char** maze;

// Numero de linhas e colunas do labirinto
int num_rows;
int num_cols;

bool achou = 0;

// Representação de uma posição
struct pos_t {
    int i;
    int j;
};

// Estrutura de dados contendo as próximas
// posições a serem exploradas no labirinto
std::stack<pos_t> valid_positions;

// Mutex para coordenar o acesso à pilha de posições válidas
std::mutex mutex;

// Mutex para coordenar o acesso à matriz do labirinto
std::mutex maze_mutex;

// Registro das posições exploradas por cada thread
std::vector<std::vector<bool>> explored_positions;

// Função que le o labirinto de um arquivo texto, carrega em 
// memória e retorna a posição inicial
pos_t load_maze(const char* file_name) {
    pos_t initial_pos;
    FILE* arq;
    // Abre o arquivo para leitura (fopen)
    arq = fopen(file_name,"r");
    // Le o numero de linhas e colunas (fscanf) 
    // e salva em num_rows e num_cols
    fscanf(arq,"%i %i", &num_rows, &num_cols);

    maze = new char*[num_rows];
    // Aloca a matriz maze (malloc)
    for (int i = 0; i < num_rows; ++i){
        // Aloca cada linha da matriz
        maze[i] = new char[num_cols];
    }

    explored_positions = std::vector<std::vector<bool>>(num_rows, std::vector<bool>(num_cols, false));

    for (int i = 0; i < num_rows; ++i) {
        for (int j = 0; j < num_cols; ++j) {
            // Le o valor da linha i+1,j do arquivo e salva na posição maze[i][j]
            fscanf(arq," %c",&maze[i][j]);
            // Se o valor for 'e' salvar o valor em initial_pos
            if(maze[i][j] == 'e'){
                initial_pos.i = i;
                initial_pos.j = j; 
            }
        }
    }
    fclose(arq);
    return initial_pos;
}

// Função que imprime o labirinto
void print_maze() {
    for (int i = 0; i < num_rows; ++i) {
        for (int j = 0; j < num_cols; ++j) {
            char print_char = maze[i][j];
            // Se a posição já foi explorada por alguma thread, marca como '.'
            if (explored_positions[i][j]) {
                print_char = '.';
            }
            printf("%c", print_char);
        }
        printf("\n");
    }
    usleep(40000);
}

// Função responsável pela navegação.
// Recebe como entrada a posição initial e retorna um booleano indicando se a saída foi encontrada
void explore() {
    while (true) {
        pos_t current_pos;
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (valid_positions.empty()) {
                // A pilha está vazia, não há mais posições a serem exploradas
                // Marcar a posição atual com '.'
                maze[current_pos.i][current_pos.j] = '.';
                return;
            }
            current_pos = valid_positions.top();
            valid_positions.pop();
        }

        // Verifica se a posição atual é a saída
        if (maze[current_pos.i][current_pos.j] == 's') {
            
            // Marcar a posição anterior como '.'
            {
                std::lock_guard<std::mutex> lock(maze_mutex);
                maze[current_pos.i][current_pos.j] = '.';
            }
            achou = 1;
            return;
        }

        // Marca a posição anterior como '.'
        if (current_pos.i > 0 && maze[current_pos.i - 1][current_pos.j] == 'o') {
            {
                std::lock_guard<std::mutex> lock(maze_mutex);
                maze[current_pos.i - 1][current_pos.j] = '.';
            }
        }
        if (current_pos.i < num_rows - 1 && maze[current_pos.i + 1][current_pos.j] == 'o') {
            {
                std::lock_guard<std::mutex> lock(maze_mutex);
                maze[current_pos.i + 1][current_pos.j] = '.';
            }
        }
        if (current_pos.j > 0 && maze[current_pos.i][current_pos.j - 1] == 'o') {
            {
                std::lock_guard<std::mutex> lock(maze_mutex);
                maze[current_pos.i][current_pos.j - 1] = '.';
            }
        }
        if (current_pos.j < num_cols - 1 && maze[current_pos.i][current_pos.j + 1] == 'o') {
            {
                std::lock_guard<std::mutex> lock(maze_mutex);
                maze[current_pos.i][current_pos.j + 1] = '.';
            }
        }

        // Marca a posição atual como 'o'
        {
            std::lock_guard<std::mutex> lock(maze_mutex);
            maze[current_pos.i][current_pos.j] = 'o';
        }

        // Limpar a tela
        std::system("clear");

        // Imprimir o labirinto
        print_maze();

        // Conta o número de caminhos possíveis a partir da posição atual
        int num_possible_paths = 0;

      
        if (current_pos.i > 0 && (maze[current_pos.i - 1][current_pos.j] == 'x' || maze[current_pos.i - 1][current_pos.j] == 's') && maze[current_pos.i - 1][current_pos.j] != '.') {
            num_possible_paths++;
        }


        if (current_pos.i < num_rows - 1 && (maze[current_pos.i + 1][current_pos.j] == 'x' || maze[current_pos.i + 1][current_pos.j] == 's') && maze[current_pos.i + 1][current_pos.j] != '.') {
            num_possible_paths++;
        }

        
        if (current_pos.j > 0 && (maze[current_pos.i][current_pos.j - 1] == 'x' || maze[current_pos.i][current_pos.j - 1] == 's') && maze[current_pos.i][current_pos.j - 1] != '.') {
            num_possible_paths++;
        }

     
        if (current_pos.j < num_cols - 1 && (maze[current_pos.i][current_pos.j + 1] == 'x' || maze[current_pos.i][current_pos.j + 1] == 's') && maze[current_pos.i][current_pos.j + 1] != '.') {
            num_possible_paths++;
        }

        // Se houver mais de um caminho possível, ocorrerá a criação de threads
        if (num_possible_paths > 1) {
            // Lista de threads para explorar caminhos alternativos
            std::vector<std::thread> threads;

        
            if (current_pos.i > 0 && (maze[current_pos.i - 1][current_pos.j] == 'x' || maze[current_pos.i - 1][current_pos.j] == 's') && maze[current_pos.i - 1][current_pos.j] != '.') {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    valid_positions.push({current_pos.i - 1, current_pos.j});
                }
                threads.emplace_back(explore); // Cria uma thread para explorar o caminho alternativo
            }

            
            if (current_pos.i < num_rows - 1 && (maze[current_pos.i + 1][current_pos.j] == 'x' || maze[current_pos.i + 1][current_pos.j] == 's') && maze[current_pos.i + 1][current_pos.j] != '.') {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    valid_positions.push({current_pos.i + 1, current_pos.j});
                }
                threads.emplace_back(explore); // Cria uma thread para explorar o caminho alternativo
            }

    
            if (current_pos.j > 0 && (maze[current_pos.i][current_pos.j - 1] == 'x' || maze[current_pos.i][current_pos.j - 1] == 's') && maze[current_pos.i][current_pos.j - 1] != '.') {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    valid_positions.push({current_pos.i, current_pos.j - 1});
                }
                threads.emplace_back(explore); // Cria uma thread para explorar o caminho alternativo
            }

        
            if (current_pos.j < num_cols - 1 && (maze[current_pos.i][current_pos.j + 1] == 'x' || maze[current_pos.i][current_pos.j + 1] == 's') && maze[current_pos.i][current_pos.j + 1] != '.') {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    valid_positions.push({current_pos.i, current_pos.j + 1});
                }
                threads.emplace_back(explore); // Cria uma thread para explorar o caminho alternativo
            }

            // Aguarda todas as threads concluírem
            for (auto& thread : threads) {
                thread.join();
            }
        } else if (num_possible_paths == 1) {
            // Caso haja apenas um caminho possível, a exploração se dará em apenas uma única thread

            if (current_pos.i > 0 && (maze[current_pos.i - 1][current_pos.j] == 'x' || maze[current_pos.i - 1][current_pos.j] == 's') && maze[current_pos.i - 1][current_pos.j] != '.') {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    valid_positions.push({current_pos.i - 1, current_pos.j});
                }
                
            }

           
            if (current_pos.i < num_rows - 1 && (maze[current_pos.i + 1][current_pos.j] == 'x' || maze[current_pos.i + 1][current_pos.j] == 's') && maze[current_pos.i + 1][current_pos.j] != '.') {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    valid_positions.push({current_pos.i + 1, current_pos.j});
                }
               
            }


            if (current_pos.j > 0 && (maze[current_pos.i][current_pos.j - 1] == 'x' || maze[current_pos.i][current_pos.j - 1] == 's') && maze[current_pos.i][current_pos.j - 1] != '.') {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    valid_positions.push({current_pos.i, current_pos.j - 1});
                }
            
            }

            if (current_pos.j < num_cols - 1 && (maze[current_pos.i][current_pos.j + 1] == 'x' || maze[current_pos.i][current_pos.j + 1] == 's') && maze[current_pos.i][current_pos.j + 1] != '.') {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    valid_positions.push({current_pos.i, current_pos.j + 1});
                }
       
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // carregar o labirinto com o nome do arquivo recebido como argumento
    pos_t initial_pos = load_maze("../data/maze6.txt");
    
    // Adicionando a posição inicial à pilha
    valid_positions.push(initial_pos);
    explore();
    if(achou){
        std::cout << "Saida encontrada" << std::endl;
    }
    else{
        std::cout << "Saida não encontrada" << std::endl;
    }

    return 0;
}
