#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#define MAX_TAREFAS 1000
#define MAX_MAQUINAS 11

typedef struct
{
    int custo;
    int precedencia[MAX_TAREFAS];
    int num_predecessores;
} Tarefa;

typedef struct Precedencia
{
    int tarefa;
    struct Precedencia *proxima;
} Precedencia;

typedef struct
{
    Precedencia *primeira;
    Precedencia *ultima;
} OrdemPrecedencia;

Tarefa tarefas[MAX_TAREFAS];
int num_tarefas;

void lerInstancias(const char *arquivo)
{
    FILE *file = fopen(arquivo, "r");
    if (file == NULL)
    {
        printf("Erro ao abrir arquivo\n");
        exit(1);
    }

    fscanf(file, "%d", &num_tarefas);

    for (int i = 0; i < num_tarefas; i++)
    {
        fscanf(file, "%d", &tarefas[i].custo);
    }

    int tarefa1, tarefa2;
    while (fscanf(file, "%d,%d", &tarefa1, &tarefa2) == 2)
    {
        if (tarefa1 == -1 && tarefa2 == -1)
            break;
        tarefas[tarefa2 - 1].precedencia[tarefas[tarefa2 - 1].num_predecessores++] = tarefa1 - 1;
    }

    fclose(file);
}

void imprimirPrecedencias(const Tarefa *tarefas, int num_tarefas)
{
    for (int i = 0; i < num_tarefas; i++)
    {
        printf("Tarefa %d: Precedências [", i + 1);
        for (int j = 0; j < tarefas[i].num_predecessores; j++)
        {
            printf("%d", tarefas[i].precedencia[j] + 1);
            if (j < tarefas[i].num_predecessores - 1)
            {
                printf(", ");
            }
        }
        printf("]\n");
    }
}

int *obterPrecedencias(int tarefa, int *num_precedencias)
{
    int *precedencias = malloc(MAX_TAREFAS * sizeof(int));
    *num_precedencias = 0;

    // Verificar as precedências diretas
    for (int i = 0; i < tarefas[tarefa].num_predecessores; i++)
    {
        int precedencia = tarefas[tarefa].precedencia[i];

        // Verificar se a precedência já está no vetor
        int repetido = 0;
        for (int j = 0; j < *num_precedencias; j++)
        {
            if (precedencias[j] == precedencia)
            {
                repetido = 1;
                break;
            }
        }

        // Adicionar a precedência somente se não estiver repetida
        if (!repetido)
        {
            precedencias[*num_precedencias] = precedencia;
            (*num_precedencias)++;

            // Chamar recursivamente para obter as precedências indiretas
            int indiretas_num_precedencias;
            int *indiretas = obterPrecedencias(precedencia, &indiretas_num_precedencias);
            for (int j = 0; j < indiretas_num_precedencias; j++)
            {
                int precedencia_indireta = indiretas[j];
                // Verificar se a precedência indireta já está no vetor
                int repetido_indireto = 0;
                for (int k = 0; k < *num_precedencias; k++)
                {
                    if (precedencias[k] == precedencia_indireta)
                    {
                        repetido_indireto = 1;
                        break;
                    }
                }

                // Adicionar a precedência indireta somente se não estiver repetida
                if (!repetido_indireto)
                {
                    precedencias[*num_precedencias] = precedencia_indireta;
                    (*num_precedencias)++;
                }
            }
            free(indiretas);
        }
    }

    return precedencias;
}

void imprimirPrecedenciasDiretasEIndiretas(const Tarefa *tarefas, int num_tarefas)
{
    for (int i = 0; i < num_tarefas; i++)
    {
        int num_precedencias;
        int *precedencias = obterPrecedencias(i, &num_precedencias);

        printf("Tarefa %d: Precedências [", i + 1);
        for (int j = 0; j < num_precedencias; j++)
        {
            printf("%d", precedencias[j] + 1);
            if (j < num_precedencias - 1)
            {
                printf(", ");
            }
        }
        printf("]\n");

        free(precedencias);
    }
}

bool todasPrecedenciasAlocadas(int tarefa, const int *precedencias, int num_precedencias, const bool *alocada)
{
    for (int i = 0; i < num_precedencias; i++)
    {
        if (!alocada[precedencias[i]])
        {
            return false;
        }
    }
    return true;
}

void atribuirTarefa(int tarefa, const int *precedencias, int num_precedencias, bool *alocada, int *ordem, int *indice_ordem, int *makespan)
{
    if (alocada[tarefa])
    {
        return;
    }

    if (!todasPrecedenciasAlocadas(tarefa, precedencias, num_precedencias, alocada))
    {
        return;
    }

    alocada[tarefa] = true;
    ordem[(*indice_ordem)++] = tarefa;

    // Atualizar makespan da máquina correspondente
    int maquina = (*indice_ordem - 1) % MAX_MAQUINAS;
    *makespan += tarefas[tarefa].custo;

    for (int i = 0; i < num_tarefas; i++)
    {
        if (tarefas[i].num_predecessores > 0)
        {
            int *precedencias_indiretas = obterPrecedencias(i, &num_precedencias);
            atribuirTarefa(i, precedencias_indiretas, num_precedencias, alocada, ordem, indice_ordem, makespan);
            free(precedencias_indiretas);
        }
    }
}

int *criarOrdemAtribuicao(const Tarefa *tarefas, int num_tarefas, int *makespan)
{
    bool alocada[MAX_TAREFAS] = {false};
    int *ordem = malloc(num_tarefas * sizeof(int));
    int indice_ordem = 0;
    *makespan = 0;

    // Embaralhar os índices das tarefas
    int indices_embaralhados[MAX_TAREFAS];
    for (int i = 0; i < num_tarefas; i++)
    {
        indices_embaralhados[i] = i;
    }
    srand(time(NULL));
    for (int i = num_tarefas - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = indices_embaralhados[i];
        indices_embaralhados[i] = indices_embaralhados[j];
        indices_embaralhados[j] = temp;
    }

    for (int i = 0; i < num_tarefas; i++)
    {
        int tarefa = indices_embaralhados[i];
        int num_precedencias;
        int *precedencias = obterPrecedencias(tarefa, &num_precedencias);
        atribuirTarefa(tarefa, precedencias, num_precedencias, alocada, ordem, &indice_ordem, makespan);
        free(precedencias);
    }

    return ordem;
}

void imprimirOrdemAtribuicao(const int *ordem, int num_tarefas)
{
    printf("Ordem de atribuição das tarefas:\n");
    for (int i = 0; i < num_tarefas; i++)
    {
        printf("%d ", ordem[i] + 1);
    }
    printf("\n");
}

void atribuirTarefasPorMaquinas(const int *ordem, int num_tarefas, int num_maquinas)
{

    int *makespanDosCria = 0;
    int tarefas_por_maquina = num_tarefas / num_maquinas;
    int tarefas_extras = num_tarefas % num_maquinas;

    FILE *file = fopen("resultado.txt", "a");
    if (file == NULL)
    {
        printf("Erro ao criar arquivo\n");
        return;
    }

    int tempos_maquinas[MAX_MAQUINAS] = {0}; // Array para armazenar os tempos de cada máquina

    fprintf(file, "Máquina 1:");
    for (int i = 0; i < tarefas_por_maquina + tarefas_extras; i++)
    {
        int tarefa = ordem[i];
        fprintf(file, " %d", tarefa + 1);
        tempos_maquinas[0] += tarefas[tarefa].custo; // Adiciona o tempo da tarefa à máquina 1
    }
    fprintf(file, "\n");

    int tarefa_inicio = tarefas_por_maquina + tarefas_extras;
    for (int i = 1; i < num_maquinas; i++)
    {
        fprintf(file, "Máquina %d:", i + 1);
        for (int j = 0; j < tarefas_por_maquina; j++)
        {
            int tarefa = ordem[tarefa_inicio];
            fprintf(file, " %d", tarefa + 1);
            tempos_maquinas[i] += tarefas[tarefa].custo; // Adiciona o tempo da tarefa à máquina i+1
            tarefa_inicio++;
        }
        fprintf(file, "\n");
    }

    int makespan = 0;
    for (int i = 0; i < num_maquinas; i++)
    {
        if (tempos_maquinas[i] > makespan)
        {
            makespan = tempos_maquinas[i]; // Encontra o maior tempo entre as máquinas
        }
    }

    fprintf(file, " \n");
    printf("Makespan: com %d máquinas:  %d\n", num_maquinas, makespan);
    fprintf(file, "Makespan: %d\n", makespan);

    fclose(file);
}

void imprimirTarefasPorMaquina(const int *ordem, int num_tarefas, int num_maquinas)
{
    int tarefas_por_maquina = num_tarefas / num_maquinas;
    int tarefas_extras = num_tarefas % num_maquinas;

    printf("Tarefas por Máquina:\n");
    for (int i = 0; i < num_maquinas; i++)
    {
        printf("Máquina %d:", i + 1);
        int tarefa_inicio = i * tarefas_por_maquina + (i < tarefas_extras ? i : tarefas_extras);
        int tarefa_fim = tarefa_inicio + tarefas_por_maquina + (i < tarefas_extras ? 1 : 0);
        for (int j = tarefa_inicio; j < tarefa_fim; j++)
        {
            printf(" %d", ordem[j] + 1);
        }
        printf("\n");
    }
}

int main()
{

    lerInstancias("instancia1.txt");

    // printf("Precedências diretas e indiretas:\n");
    // imprimirPrecedenciasDiretasEIndiretas(tarefas, num_tarefas);

    int makespan;
    int *ordem = criarOrdemAtribuicao(tarefas, num_tarefas, &makespan);

    // imprimirOrdemAtribuicao(ordem, num_tarefas);

    for (int i = 3; i <= MAX_MAQUINAS; i++)
    {
        imprimirTarefasPorMaquina(ordem, num_tarefas, i);
        atribuirTarefasPorMaquinas(ordem, num_tarefas, i);

        printf("\n");
    }

    free(ordem);

    return 0;
}
