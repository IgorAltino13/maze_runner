#include <stdio.h>
#include <stack>
#include <fstream>
#include <iostream>
#include <unistd.h>

// Matriz de char representnado o labirinto
char** maze; // Voce também pode representar o labirinto como um vetor de vetores de char (vector<vector<char>>)

// Numero de linhas e colunas do labirinto
int num_rows;
int num_cols;

// Representação de uma posição
struct pos_t {
	int i;
	int j;
};

// Estrutura de dados contendo as próximas
// posicões a serem exploradas no labirinto
std::stack<pos_t> valid_positions;
/* Inserir elemento: 

	 pos_t pos;
	 pos.i = 1;
	 pos.j = 3;
	 valid_positions.push(pos)
 */
// Retornar o numero de elementos: 
//    valid_positions.size();
// 
// Retornar o elemento no topo: 
//  valid_positions.top(); 
// 
// Remover o primeiro elemento do vetor: 
//    valid_positions.pop();


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
			printf("%c", maze[i][j]);
		}
		printf("\n");
	}
	usleep(40000);
}


// Função responsável pela navegação.
// Recebe como entrada a posição initial e retorna um booleando indicando se a saída foi encontrada
bool walk(pos_t pos) {
    // Verificar se a posição atual é a saída
    if (maze[pos.i][pos.j] == 's') {
        return true;
    }

    
    maze[pos.i][pos.j] = 'o';

    // Limpar a tela
    std::system("clear");

    // Imprime o labirinto
    print_maze();

    // Dado a posição atual, verifica quais são as próximas posições válidas
    // e insere cada uma delas na pilha valid_positions
    if (pos.i > 0 && (maze[pos.i - 1][pos.j] == 'x' || maze[pos.i - 1][pos.j] == 's') && maze[pos.i - 1][pos.j] != '.') {
        valid_positions.push({pos.i - 1, pos.j});
    }
    if (pos.i < num_rows - 1 && (maze[pos.i + 1][pos.j] == 'x' || maze[pos.i + 1][pos.j] == 's') && maze[pos.i + 1][pos.j] != '.') {
        valid_positions.push({pos.i + 1, pos.j});
    }
    if (pos.j > 0 && (maze[pos.i][pos.j - 1] == 'x' || maze[pos.i][pos.j - 1] == 's') && maze[pos.i][pos.j - 1] != '.') {
        valid_positions.push({pos.i, pos.j - 1});
    }
    if (pos.j < num_cols - 1 && (maze[pos.i][pos.j + 1] == 'x' || maze[pos.i][pos.j + 1] == 's') && maze[pos.i][pos.j + 1] != '.') {
        valid_positions.push({pos.i, pos.j + 1});
    }
	// Marcar a posição atual com o símbolo '.'
	maze[pos.i][pos.j] = '.';

    
    while (!valid_positions.empty()) {
        pos_t proxima_pos = valid_positions.top();
        valid_positions.pop();
        if (walk(proxima_pos)) {
            return true;  
        }
    }

    return false;
}
int main(int argc, char* argv[]) {
	// carregar o labirinto com o nome do arquivo recebido como argumento
	pos_t initial_pos = load_maze("../data/maze.txt");
	// chamar a função de navegação
	
	bool exit_found = walk(initial_pos);
	
	
	if(exit_found){
		std::cout<<"Saida encontrada";
	}
	else{
		std::cout<<"Saida nao encontrada";
	}
	// Tratar o retorno (imprimir mensagem)
	
	return 0;
}
