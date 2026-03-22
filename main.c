#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>



// representa um vetor de movimento (direção e velocidade)
typedef struct {

    int dx, dy;
} Vector;

// representa um barco na simulação
typedef struct {

    char id;               // identificador do barco (letra)
    int x, y;              // posição atual
    int angle;             // angulo de movimento
    int speed;             // velocidade
    int type;              // tipo do barco (0: normal, 1: drone, 2: submarino, 3: veleiro)
    Vector velocity;       // vetor de velocidade
    int visible;           // visibilidade (usado para submarinos)
    int original_x, original_y; // posição inicial
    int initial_frame;     // frame em que foi inserido
} Boat;

// nó de lista ligada para armazenar barcos
typedef struct BoatNode {

    Boat boat;
    struct BoatNode *next;
} BoatNode;

// estrutura para guardar o estado de um barco no histórico
typedef struct {

    char id;
    int x, y;
    int angle;
    int speed;
    int type;
} HistoryBoat;

// nó de lista ligada para histórico de frames
typedef struct HistoryNode {

    HistoryBoat *boats;    // array de barcos neste frame
    int num_boats;         // quantos barcos havia
    int frame_number;      // número do frame
    int current_angle;     // angulo da corrente neste frame
    int current_speed;     // velocidade da corrente neste frame
    struct HistoryNode *next;
} HistoryNode;

// Estrutura principal da simulação
typedef struct {

    int grid_width, grid_height; // dimensões do mapa
    int current_angle;           // angulo da corrente
    int current_speed;           // velocidade da corrente
    int current_frame;           // frame atual
    BoatNode *boat_list;         // lista ligada de barcos
    HistoryNode *history;        // lista ligada de histórico
} Simulation;



// converte um ângulo e velocidade num vetor de movimento
Vector angle_to_vector(int angle, int speed) {

    Vector v = {0, 0};

    switch(angle) {

        case 0:   v.dx = speed; break;
        case 45:  v.dx = speed; v.dy = speed; break;
        case 90:  v.dy = speed; break;
        case 135: v.dx = -speed; v.dy = speed; break;
        case 180: v.dx = -speed; break;
        case 225: v.dx = -speed; v.dy = -speed; break;
        case 270: v.dy = -speed; break;
        case 315: v.dx = speed; v.dy = -speed; break;
    }

    return v;
}

// Converte um vetor de movimento num ângulo correspondente
int vector_to_angle(Vector v) {

    if (v.dx > 0 && v.dy == 0) return 0;
    if (v.dx > 0 && v.dy > 0) return 45;
    if (v.dx == 0 && v.dy > 0) return 90;
    if (v.dx < 0 && v.dy > 0) return 135;
    if (v.dx < 0 && v.dy == 0) return 180;
    if (v.dx < 0 && v.dy < 0) return 225;
    if (v.dx == 0 && v.dy < 0) return 270;
    if (v.dx > 0 && v.dy < 0) return 315;
    return 0;
}

// verifica se o ângulo é válido (multiplo de 45)
bool is_valid_angle(int angle) {

    int valid_angles[] = {0, 45, 90, 135, 180, 225, 270, 315};

    for (int i = 0; i < 8; i++) {

        if (angle == valid_angles[i]) return true;
    }
    return false;
}



// cria um novo nó de barco
BoatNode *create_boat_node(Boat boat) {

    BoatNode *node = (BoatNode*)malloc(sizeof(BoatNode));
    if (!node) return NULL;
    node->boat = boat;
    node->next = NULL;
    return node;
}

// adiciona ou atualiza um barco na lista
void add_boat(Simulation *sim, Boat boat) {

    // cria novo nó para o barco
    BoatNode *new_node = create_boat_node(boat);
    if (!new_node) return; // se não conseguiu alocar memória da return

    BoatNode *current = sim->boat_list; // ponteiro para percorrer a lista ligada
    BoatNode *prev = NULL; // ponteiro para o nó anterior

    // percorre a lista ligada para procurar barco com o mesmo id

    while (current) {   // inicio do ciclo de procura de barco existente

        if (current->boat.id == boat.id) {

            // Se encontrou, substitui o nó antigo pelo novo
            new_node->next = current->next;
            if (prev) {
                prev->next = new_node;
            } else {
                sim->boat_list = new_node;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    } // fim do ciclo de procura

    // se não encontrou, insere o novo nó no início da lista
    new_node->next = sim->boat_list;
    sim->boat_list = new_node;
}

// remove um barco da lista pelo id
void remove_boat(Simulation *sim, char id) {

    BoatNode *current = sim->boat_list;
    BoatNode *prev = NULL;

    // percorre a lista ligada para encontrar o barco a remover
    while (current) { // inicio ciclo de remoção

        if (current->boat.id == id) {
            // remove o nó correspondente
            if (prev) {
                prev->next = current->next;
            } else {
                sim->boat_list = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    } // fim do ciclo de remoção
}

// procura um barco pelo id
Boat *find_boat(Simulation *sim, char id) {

    BoatNode *current = sim->boat_list;
    // percorre a lista ligada para encontrar o barco
    while (current) { // inicio do ciclo de procura

        if (current->boat.id == id) {
            return &current->boat;
        }
        current = current->next;
    } // fim do ciclo de procura
    return NULL;
}



// salva o estado atual da simulação no histórico
void save_frame_to_history(Simulation *sim) {

    // aloca a memória para um novo nó do histórico da simulação
    HistoryNode *hist_node = (HistoryNode*)malloc(sizeof(HistoryNode));
    if (!hist_node) return;

    // conta quantos barcos existem atualmente
    int count = 0;
    BoatNode *current = sim->boat_list;
    while (current) { // inicio do ciclo de contagem de barcos
        count++;
        current = current->next;
    } // fim do ciclo de contagem

    hist_node->boats = (HistoryBoat*)malloc(sizeof(HistoryBoat) * count);
    if (!hist_node->boats) {

        free(hist_node);
        return;
    }

    hist_node->num_boats = count;
    hist_node->frame_number = sim->current_frame;
    hist_node->current_angle = sim->current_angle;
    hist_node->current_speed = sim->current_speed;

    // copia o estado de cada barco para o histórico
    int i = 0;
    current = sim->boat_list;
    while (current) { // inicio do ciclo de cópia para histórico

        // Copia os dados do barco atual para o histórico
        hist_node->boats[i].id = current->boat.id;
        hist_node->boats[i].x = current->boat.x;
        hist_node->boats[i].y = current->boat.y;
        hist_node->boats[i].angle = current->boat.angle;
        hist_node->boats[i].speed = current->boat.speed;
        hist_node->boats[i].type = current->boat.type;
        i++;
        current = current->next;
    } // fim do ciclo de cópia

    // adiciona o novo frame ao início da lista de histórico
    hist_node->next = sim->history;
    sim->history = hist_node;
}

// procura um frame específico no histórico
HistoryNode *find_history_frame(Simulation *sim, int frame) {

    HistoryNode *current = sim->history;
    // percorre a lista ligada de histórico para encontrar o frame
    while (current) { // inicio do ciclo de procura no histórico
        if (current->frame_number == frame) { // se encontrou o frame retorna o nó
            return current;
        }
        current = current->next;
    } // fim do ciclo de procura
    return NULL;
}



// atualiza a posição de um barco conforme o seu tipo e a corrente fornecida
void update_boat_position(Simulation *sim, Boat *boat) {

    Vector current_vector = angle_to_vector(sim->current_angle, sim->current_speed);

    switch (boat->type) {

        case 0: // Normal
            // Movimento normal: soma vetor próprio e da corrente
            boat->x += boat->velocity.dx + current_vector.dx;
            boat->y += boat->velocity.dy + current_vector.dy;
            break;
        case 1: { // Drone
            // movimento alternado no horizontal em frames ímpares, vertical em pares

            if (sim->current_frame % 2 == 1) {

                boat->x += boat->velocity.dx + current_vector.dx;
            } else {
                boat->y += boat->velocity.dy + current_vector.dy;
            }
            // verifica limites e inverte direção se necessário
            if (boat->x < 0) {

                boat->x = 0;
                boat->velocity.dx = -boat->velocity.dx;
                boat->angle = vector_to_angle(boat->velocity);
            } else if (boat->x >= sim->grid_width) {

                boat->x = sim->grid_width - 1;
                boat->velocity.dx = -boat->velocity.dx;
                boat->angle = vector_to_angle(boat->velocity);
            }
            if (boat->y < 0) {

                boat->y = 0;
                boat->velocity.dy = -boat->velocity.dy;
                boat->angle = vector_to_angle(boat->velocity);
            } else if (boat->y >= sim->grid_height) {
                boat->y = sim->grid_height - 1;
                boat->velocity.dy = -boat->velocity.dy;
                boat->angle = vector_to_angle(boat->velocity);
            }
            break;
        }
        case 2: // Submarino
            // movimento normal e altera alternância de visibilidade
            boat->x += boat->velocity.dx + current_vector.dx;
            boat->y += boat->velocity.dy + current_vector.dy;
            boat->visible = (sim->current_frame % 10 < 5);
            break;
        case 3: { // Veleiro
            // movimento especial em que dobra dx se for para a direita
            int new_x = boat->x + boat->velocity.dx + current_vector.dx;
            int new_y = boat->y + boat->velocity.dy + current_vector.dy;
            if (boat->velocity.dx > 0) {
                new_x = boat->x + (boat->velocity.dx * 2) + current_vector.dx;
            }
            // se sair dos limites horizontais inverte a direção
            if (new_x < 0 || new_x >= sim->grid_width) {
                boat->velocity.dx = -boat->velocity.dx;
                boat->velocity.dy = -boat->velocity.dy;
                boat->angle = vector_to_angle(boat->velocity);
            } else {
                boat->x = new_x;
                boat->y = new_y;
            }
            break;
        }
    }
}

// atualiza a simulação por um certo número de frames
void update_simulation(Simulation *sim, int frames) {

    for (int f = 0; f < frames; f++) { // inicio ciclo de avanço de frames
        sim->current_frame++;

        // atualiza posições de todos os barcos
        BoatNode *current = sim->boat_list;
        while (current) { // incio ciclo de atualizaçãode posição dos barcos

            update_boat_position(sim, &current->boat);
            current = current->next;
        } // fim do ciclo de atualização

        // detecta e resolve colisões entre barcos
        BoatNode *boat1 = sim->boat_list;
        while (boat1) { // inicio do ciclo externo de colisão
            BoatNode *boat2 = boat1->next;
            while (boat2) { // inccio do ciclo interno de colisão
                if (boat1->boat.x == boat2->boat.x && boat1->boat.y == boat2->boat.y) {

                    // lógica de colisão
                    if (boat1->boat.type == 2 && !boat1->boat.visible) {

                        remove_boat(sim, boat2->boat.id);
                        boat2 = boat1->next;
                        continue;
                    } else if (boat2->boat.type == 2 && !boat2->boat.visible) {

                        remove_boat(sim, boat1->boat.id);
                        boat1 = sim->boat_list;
                        break;
                    } else {

                        char id1 = boat1->boat.id;
                        char id2 = boat2->boat.id;
                        remove_boat(sim, id1);
                        remove_boat(sim, id2);
                        boat1 = sim->boat_list;
                        break;
                    }
                }
                boat2 = boat2->next;
            } // fim do ciclo interno de colisão
            if (boat1)
                boat1 = boat1->next;
        } // fim do ciclo externo de colisão

        // 3) Remover barcos fora dos limites do mapa
        BoatNode *current_boat = sim->boat_list;
        BoatNode *prev = NULL;

        while (current_boat) { // incio do ciclo de remoção de barcos fora dos limites

            int out_x = (current_boat->boat.x < 0 || current_boat->boat.x >= sim->grid_width); // verifica se está fora dos limites horizontais
            int out_y = (current_boat->boat.y < 0 || current_boat->boat.y >= sim->grid_height); // verifica se está fora dos limites verticais
            int should_remove = out_y || (out_x && current_boat->boat.type != 3); // Veleiros não são removidos se saírem horizontalmente
            if (should_remove) {

                BoatNode *to_remove = current_boat;
                if (prev) {

                    prev->next = current_boat->next;
                    current_boat = current_boat->next;
                } else {

                    sim->boat_list = current_boat->next;
                    current_boat = sim->boat_list;
                }
                free(to_remove);
            } else {
                prev = current_boat;
                current_boat = current_boat->next;
            }
        } // fim do ciclo de remoção

        // 4) salva estado no histórico após cada frame
        save_frame_to_history(sim);
    } // fim do ciclo de avanço de frames
}



// carrega barcos de um ficheiro de texto
void load_boats_from_file(Simulation *sim, const char *filename) {

    FILE *file = fopen(filename, "r"); // Abre o ficheiro para leitura
    if (!file) return;

    char id;
    int x, y, angle, speed, type; // Variáveis para ler os dados do barco

    // Lê cada linha do ficheiro e adiciona barco à simulação
    while (fscanf(file, " %c %d %d %d %d %d", &id, &x, &y, &angle, &speed, &type) == 6) { // INÍCIO ciclo de leitura do ficheiro

        Boat boat;
        boat.id = id;
        boat.x = x;
        boat.y = y;
        boat.original_x = x;
        boat.original_y = y;
        boat.angle = angle;
        boat.speed = speed;
        boat.type = type;
        boat.velocity = angle_to_vector(angle, speed);
        boat.visible = 1;
        boat.initial_frame = sim->current_frame;
        add_boat(sim, boat);
    } // fim do ciclo de leitura
    fclose(file);
}

// Salva o estado atual dos barcos num ficheiro
void save_boats_to_file(Simulation *sim, const char *filename) {

    FILE *file = fopen(filename, "w");
    if (!file) return;

    // Conta quantos barcos existem e são visíveis
    int count = 0;
    BoatNode *current = sim->boat_list;
    while (current) { // inicio do  ciclo de contagem de barcos visíveis
        if (current->boat.type != 2 || current->boat.visible) {

            count++;
        }
        current = current->next;
    } // fim do ciclo de contagem

    // Copia barcos para array temporário
    Boat *boats_array = (Boat*)malloc(sizeof(Boat) * count); // aloca memória para array de barcos
    if (!boats_array) { // verifica se a alocação foi bem sucedida
        fclose(file);
        return;
    }

    int index = 0;
    current = sim->boat_list;
    while (current) { // inicio do ciclo de cópia para array
        if (current->boat.type != 2 || current->boat.visible) { // verifica se o barco é visível ou não é submarino
            boats_array[index] = current->boat; // copia o barco para o array
            index++;
        }
        current = current->next;
    } // fim do ciclo de cópia

    // Grava barcos no ficheiro em ordem inversa
    for (int i = count - 1; i >= 0; i--) { // inicio do ciclo de gravação no ficheiro
        fprintf(file, "%c %d %d %d %d %d\n", // grava cada barco
                boats_array[i].id,
                boats_array[i].x,
                boats_array[i].y,
                boats_array[i].angle,
                boats_array[i].speed,
                boats_array[i].type);
    } // fim do ciclo de gravação

    free(boats_array);
    fclose(file);
    printf("Frame %d guardado com sucesso em %s\n", sim->current_frame, filename);
}


// previsão de colisões futuras entre barcos
void predict_collisions(Simulation *sim) {

    printf("=== Previsao de Colisoes ===\n");
    int collisions = 0;

    BoatNode *boat1 = sim->boat_list;
    while (boat1) { // inicio do ciclo externo de pares de barcos

        BoatNode *boat2 = boat1->next;
        while (boat2) { // inicio ciclo interno de pares

            // simula até 1000 frames à frente para prever colisão
            for (int t = 1; t <= 1000; t++) { // inicio ciclo de simulação futura

                int x1 = boat1->boat.x + (boat1->boat.velocity.dx * t); // calcula posição futura do barco 1
                int y1 = boat1->boat.y + (boat1->boat.velocity.dy * t); // calcula posição futura do barco 1
                int x2 = boat2->boat.x + (boat2->boat.velocity.dx * t); // calcula posição futura do barco 2
                int y2 = boat2->boat.y + (boat2->boat.velocity.dy * t); // calcula posição futura do barco 2

                if (x1 == x2 && y1 == y2 &&
                    x1 >= 0 && x1 < sim->grid_width &&
                    y1 >= 0 && y1 < sim->grid_height) {
                    printf("Colisao prevista entre barcos %c e %c:\n", boat1->boat.id, boat2->boat.id);
                    printf("   Posicao prevista da colisao: (%d,%d)\n", x1, y1);
                    collisions++;
                    break;
                }
            } // fim do ciclo de simulação futura
            boat2 = boat2->next;
        } // fim do ciclo interno de pares
        boat1 = boat1->next;
    } // fim do ciclo externo de pares

    printf("Total de colisoes previstas: %d\n", collisions);
}

// volta atrás no histórico um certo número de frames
void reverse_history(Simulation *sim, int frames_back) {

    if (frames_back > sim->current_frame) {
        frames_back = sim->current_frame;
    }
    int target_frame = sim->current_frame - frames_back;
    HistoryNode *hist = find_history_frame(sim, target_frame);

    if (!hist) {
        printf("Frame %d nao encontrado no historico.\n", target_frame);
        return;
    }

    // Restaurar corrente
    sim->current_angle = hist->current_angle;
    sim->current_speed = hist->current_speed;

    // remove todos os barcos atuais
    while (sim->boat_list) { // inicio do ciclo de remoção de barcos atuais

        BoatNode *temp = sim->boat_list;
        sim->boat_list = sim->boat_list->next;
        free(temp);
    } // fim do ciclo de remoção

    // repõe barcos do histórico
    for (int i = 0; i < hist->num_boats; i++) { // inicio do ciclo de reposição de barcos

        // cria um novo barco a partir do histórico
        HistoryBoat hb = hist->boats[i];
        Boat boat;
        boat.id = hb.id;
        boat.x = hb.x;
        boat.y = hb.y;
        boat.original_x = hb.x;
        boat.original_y = hb.y;
        boat.angle = hb.angle;
        boat.speed = hb.speed;
        boat.type = hb.type;
        boat.velocity = angle_to_vector(hb.angle, hb.speed);
        boat.visible = 1;
        boat.initial_frame = target_frame;
        add_boat(sim, boat);
    } // fim do ciclo de reposição

    sim->current_frame = target_frame;

    printf("=== Rastrear Historico Reverso ===\n");
    printf("Estado do frame %d:\n", target_frame);

    BoatNode *current = sim->boat_list;
    while (current) { // inicia o ciclo de apresentação dos barcos restaurados
        printf("Barco %c: posicao (%d,%d), velocidade (%d,%d)\n",
               current->boat.id, current->boat.x, current->boat.y, // apresenta o barco
               current->boat.velocity.dx, current->boat.velocity.dy); // apresenta a velocidade
        current = current->next;
    } // fim do ciclo de apresentação
}

// calcula a velocidade média de um barco desde a sua inserção
void average_speed(Simulation *sim) {

    printf("=== Velocidade Media de um Barco ===\n");
    printf("Nome do barco (uma letra): ");
    char id;
    scanf(" %c", &id);

    Boat *boat = find_boat(sim, id); // procura o barco pelo id

    if (!boat) {

        printf("Barco nao encontrado.\n");
        return;
    }

    int frames_active = sim->current_frame - boat->initial_frame; // calcula quantos frames o barco está ativo

    if (frames_active <= 0) { // se o barco não se moveu ainda

        printf("Barco ainda nao se moveu.\n");
        return;
    }

    int dx = boat->x - boat->original_x;
    int dy = boat->y - boat->original_y;
    double distance = sqrt(dx * dx + dy * dy); // calcula a distância percorrida
    double avg_speed = distance / frames_active; // velocidade média

    printf("Estatisticas do barco %c:\n", id);
    printf(" Posicao inicial: (%d,%d)\n", boat->original_x, boat->original_y);
    printf(" Posicao atual: (%d,%d)\n", boat->x, boat->y);
    printf(" Distancia percorrida: %.2f casas\n", distance);
    printf(" Numero de frames: %d\n", frames_active);
    printf(" Velocidade media: %.2f casas/frame\n", avg_speed);
}

// permite alterar a corrente (velocidade e ângulo)
void change_current(Simulation *sim) {

    printf("=== Alterar Corrente ===\n");
    int new_speed, new_angle; // variáveis para nova velocidade e ângulo

    while (1) {

        printf("Nova velocidade da corrente: ");
        if (scanf("%d", &new_speed) != 1 || new_speed < 0) { // verifica se é um número válido

            printf("Valor invalido. Digite um numero inteiro positivo.\n");
            while (getchar() != '\n'); // limpar buffer
            continue;
        }
        break;
    }

    while (1) {
        printf("Novo angulo da corrente (multiplo de 45): ");

        if (scanf("%d", &new_angle) != 1 || !is_valid_angle(new_angle)) { // verifica se é um ângulo válido

            printf("Angulo invalido. Use um multiplo de 45 (0,45,90,135,180,225,270,315).\n");
            while (getchar() != '\n'); // limpar buffer
            continue;
        }
        break;
    }

    sim->current_speed = new_speed;
    sim->current_angle = new_angle;
    printf("Corrente alterada para velocidade %d e angulo %d.\n", new_speed, new_angle);
}

// Permite inserir um novo barco ou alterar um existente
void insert_or_update_boat(Simulation *sim) {

    printf("=== Inserir/Alterar Barco ===\n");

    char id;
    int x, y, angle, speed, type;

    // nome do barco
    while (1) {

        printf("Nome do barco (uma letra): ");

        if (scanf(" %c", &id) != 1 || !isalpha(id)) {

            printf("ID invalido. Use uma unica letra.\n");
            while (getchar() != '\n'); // limpar buffer
            continue;
        }
        break;
    }

    // posição inicial
    while (1) {

        printf("Posicao inicial (latitude longitude): ");

        if (scanf("%d %d", &x, &y) != 2 || x < 0 || y < 0) {

            printf("Valores inválidos. Use coordenadas positivas.\n");
            while (getchar() != '\n'); // limpar buffer
            continue;
        }
        break;
    }

    // angulo
    while (1) {

        printf("Angulo (multiplo de 45): ");

        if (scanf("%d", &angle) != 1 || !is_valid_angle(angle)) {

            printf("Angulo invalido. Use um multiplo de 45 (0,45,90,135,180,225,270,315).\n");
            while (getchar() != '\n'); // limpar buffer
            continue;
        }
        break;
    }

    // velocidade
    while (1) {
        printf("Velocidade: ");

        if (scanf("%d", &speed) != 1 || speed < 0) { // verifica se é um número válido

            printf("Valor invalido. Digite um numero inteiro positivo.\n");
            while (getchar() != '\n'); // limpar buffer
            continue;
        }
        break;
    }

    // tipo do barco
    while (1) {

        printf("Tipo do barco (0-3): ");

        if (scanf("%d", &type) != 1 || type < 0 || type > 3) { // verifica se é um número válido entre 0 e 3

            printf("Tipo invalido. Use 0, 1, 2 ou 3.\n");
            while (getchar() != '\n'); // limpa buffer
            continue;
        }
        break;
    }

    // cria o barco com os dados fornecidos
    Boat boat;
    boat.id = id;
    boat.x = x;
    boat.y = y;
    boat.original_x = x;
    boat.original_y = y;
    boat.angle = angle;
    boat.speed = speed;
    boat.type = type;
    boat.velocity = angle_to_vector(angle, speed);
    boat.visible = 1;
    boat.initial_frame = sim->current_frame;

    if (find_boat(sim, id)) { // se já existe, atualiza os dados

        printf("Barco %c alterado com sucesso.\n", id);
    } else { // se não existe, adiciona novo barco

        printf("Barco %c adicionado com sucesso.\n", id);
    }

    add_boat(sim, boat);
}

// mostra o menu da simulação
void show_menu() {

    printf("\n=== MENU DA SIMULACAO ===\n");
    printf("1. Atualizar simulacao\n");
    printf("2. Inserir ou alterar barco\n");
    printf("3. Previsao de colisoes\n");
    printf("4. Rastrear historico reverso\n");
    printf("5. Velocidade media de um barco\n");
    printf("6. Alterar corrente\n");
    printf("0. Sair\n");
    printf("Escolha uma opcao: ");
}

// liberta toda a memória alocada pela simulação
void free_memory(Simulation *sim) {

    // liberta lista de barcos
    while (sim->boat_list) { // INÍCIO ciclo de libertação de barcos

        BoatNode *temp = sim->boat_list;
        sim->boat_list = sim->boat_list->next;
        free(temp);
    } // fim do ciclo de libertação

    // liberta o histórico
    while (sim->history) { // inicio do ciclo de libertação do histórico

        HistoryNode *temp = sim->history;
        sim->history = sim->history->next;
        free(temp->boats);
        free(temp);
    } // fim do ciclo de libertação
}



int main(int argc, char *argv[]) {

    if (argc != 7) {

        printf("Uso: %s <arquivo_entrada> <dimensao> <angulo_corrente> <velocidade_corrente> <frames> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    // começa a simulação
    Simulation sim = {0};

    // faz o parsing dos parâmetros
    char *input_file = argv[1];
    sscanf(argv[2], "%dx%d", &sim.grid_width, &sim.grid_height);
    sim.current_angle = atoi(argv[3]);
    sim.current_speed = atoi(argv[4]);
    int auto_frames = atoi(argv[5]);
    char *output_file = argv[6];
    sim.current_frame = 0;

    // valida o ângulo da corrente
    if (!is_valid_angle(sim.current_angle)) {

        printf("Angulo de corrente invalido. Use um multiplo de 45.\n");
        return 1;
    }

    // carrega os barcos do antes.txt
    load_boats_from_file(&sim, input_file);
    save_frame_to_history(&sim);


    // executa frames automáticos, e vai salvando cada frame
    if (auto_frames > 0) {
        for (int i = 0; i < auto_frames; i++) { // incio do ciclo de frames automáticos
            update_simulation(&sim, 1);
            save_boats_to_file(&sim, output_file);
        } // fim do ciclo de frames automáticos
        printf("Simulacao atualizada para o frame %d\n", sim.current_frame);
    }

    // menu
    int option;
    do { // inicia o ciclo principal do menu
        show_menu();
        if (scanf("%d", &option) != 1) {

            printf("Entrada invalida!\n");
            while (getchar() != '\n'); // Limpar buffer
            continue;
        }

        switch (option) {

            case 1: {

                int frames;
                printf("Quantos frames deseja avancar? ");

                if (scanf("%d", &frames) != 1 || frames <= 0) {

                    printf("Numero invalido de frames!\n");
                    break;
                }
                for (int i = 0; i < frames; i++) { // começa ciclo de avanço manual de frames

                    // avança exatamente 1 frame
                    update_simulation(&sim, 1);
                    // grava o estado desse frame
                    save_boats_to_file(&sim, output_file);
                } // fim do ciclo de avanço manual
                // no fim informa o frame atual
                printf("Simulacao atualizada para o frame %d\n", sim.current_frame);

                break;
            }
            case 2:
                insert_or_update_boat(&sim);
                break;
            case 3:
                predict_collisions(&sim);
                break;
            case 4: {

                int frames_back;
                printf("Quantos frames deseja voltar? ");

                if (scanf("%d", &frames_back) != 1 || frames_back < 0) {

                    printf("Numero invalido de frames!\n");
                    break;
                }
                reverse_history(&sim, frames_back);
                break;
            }
            case 5:
                average_speed(&sim);
                break;
            case 6:
                change_current(&sim);
                break;
            case 0:
                save_boats_to_file(&sim, output_file);
                printf("A sair do programa...\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }
    } while (option != 0); //fecha o programa

    free_memory(&sim);
    return 0;
}