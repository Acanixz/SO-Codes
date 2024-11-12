/*
* Disciplina: Organiza��o de Computadores
* Atividade : Avalia��o 02
*
* Grupo:
* - Cassiano de Sena Crispim
* - H�rick Vitor Vieira Bittencourt
* - Eduardo Miguel Fuchs Perez
*
* OBSERVA��O:
* - Devido o uso de for loops com l�gica [key, value], o padr�o ISO para
* este projeto deve ser o C++ 17, caso contr�rio a build ir� falhar
* (v� na aba projeto->propriedades de main no visual studio 2019)
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include "calculos.h"
using namespace std;


// Armazena informa��es geradas pelo binary dump
struct LinhaASM {
    // Instru��o completa (32 bits)
    string instrucao;

    // Substrings da instru��o completa
    string opcode;
    string rd;
    string funct3;
    string rs1;
    string rs2;
    string funct7;

    // Variaveis informativas
    string tipoInstrucao;
    bool isManualNOP = false;
};

// Mostra na tela todas as instrucoes do programa
// (al�m de poder destacar NOPs se preferido)
void VisualizarInstrucoes(vector<LinhaASM> programa, bool destacarNOPs = true) {
    cout << "----------------------" << endl;

    for (int i = 0; i < programa.size(); i++) {
        cout << (i + 1) << ": " << programa[i].instrucao;
        if (destacarNOPs) {
            if (programa[i].isManualNOP) {
                cout << " | NOP |";
            }
        }
        cout << endl;
    }

    cout << "----------------------" << endl;
}

void contarNOPs(vector<LinhaASM> programa) {
    int NOPs = 0;
    for (int i = 0; i < programa.size(); i++) {
        if (programa[i].isManualNOP) {
            NOPs++;
        }
    }
    cout << "TOTAL DE NOPs inseridos: " << NOPs << endl;
}

void salvarPrograma(vector<LinhaASM> programa, string nomePrograma) {
    ofstream arquivoFinal(nomePrograma);

    // Verifica��o de abertura do arquivo
    if (!arquivoFinal.is_open()) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo " + nomePrograma + " para escrita");
        return;
    }

    // Transferencia das instru��es do vetor para o arquivo
    for (LinhaASM linha : programa) {
        arquivoFinal << linha.instrucao << endl;
    }

    cout << "Instrucoes salvas com exito em " << nomePrograma << endl;
    arquivoFinal.close();
}

// Uma organiza��o, utilizado para gerar estatisticas com um vetor de LinhaASM
struct Organizacao {
    float TClock; // Tempo de clock
    float freqClock; // Frequencia de clock (1/TClock)
    map<string, float> quantCiclos; // Quantos ciclos leva cada instru��o?


    Organizacao() {
        quantCiclos["U"] = 1.f;
        quantCiclos["J"] = 1.f;
        quantCiclos["B"] = 1.f;
        quantCiclos["I_ar"] = 1.f; // Quant. Ciclos p/ instru��o Imm. Aritmetico e ecall
        quantCiclos["I_lo"] = 1.f; // Quant. Ciclos p/ instru��o Imm. Load
        quantCiclos["R"] = 1.f;
        quantCiclos["S"] = 1.f;
    }
};

// Resultados de desempenho da organiza��o com um binary dump
struct Resultados {
    float CiclosTotais; // Ciclos totais gastos
    float CPI; // Ciclos por instru��o
    float TExec; // Tempo de execu��o da CPU (Quant. instrucoes * CPI * Clock)
    double desempenho; // Float de desempenho (1.0 / TExec)
};

// Abre o arquivo, redundante, mas quem sabe
// ganha um proposito melhor depois
bool abrirArquivo(ifstream& saida, string caminho) {
    saida.open(caminho);
    if (saida.is_open()) {
        return true;
    }
    cout << "Nao foi possivel abrir o arquivo: " << caminho << endl;
    return false;
}

string lerOpcode(string opcode) {
    if (opcode == "0110111" || opcode == "0010111")
        return "U";

    if (opcode == "1101111")
        return "J";

    if (opcode == "1100011")
        return "B";

    if (opcode == "1100111" || opcode == "0010011" || opcode == "0001111" || opcode == "1110011")
        return "I_ar";

    if (opcode == "0000011")
        return "I_lo";

    if (opcode == "0110011")
        return "R";

    if (opcode == "0100011")
        return "S";

    cout << "(aviso) opcode desconhecido: " << opcode << endl;
    return "?";
}

bool verificarHazardInstrucao(LinhaASM instrucaoOrigem, LinhaASM instrucaoJ, bool forwardingImplementado) {

    // A instru��o tipo S n�o faz nenhuma escrita, logo, as proximas linhas n�o ir�o ter problemas de dependencia
    if (instrucaoOrigem.tipoInstrucao == "S") return false;

    // NO-Operators, ignora
    if (instrucaoOrigem.isManualNOP) return false;
    if (instrucaoJ.isManualNOP) return false;

    // Ignora ecalls
    if (instrucaoJ.instrucao == "00000000000000000000000001110011") return false;

    // Se forwardingImplementado, apenas procura por hazards cujo a instru��o origem � lw
    if (forwardingImplementado && instrucaoOrigem.tipoInstrucao != "I_lo") return false;

    if (instrucaoJ.tipoInstrucao == "R" || instrucaoJ.tipoInstrucao == "I_ar" || instrucaoJ.tipoInstrucao == "I_lo" || instrucaoJ.tipoInstrucao == "S" || instrucaoJ.tipoInstrucao == "B") {
        if (instrucaoOrigem.rd == instrucaoJ.rs1) {
            cout << "RD de origem conflita com rs1!" << endl;
            return true;
        }
    }

    if (instrucaoJ.tipoInstrucao == "R" || instrucaoJ.tipoInstrucao == "S" || instrucaoJ.tipoInstrucao == "B") {
        if (instrucaoOrigem.rd == instrucaoJ.rs2) {
            cout << "RD de origem conflita com rs2!" << endl;
            return true;
        }
    }
    return false;
}

// Insere um NOP a frente de cada jump, s� pra ter certeza
vector<LinhaASM> inserirNOPsEmJump(vector<LinhaASM> instrucoes) {
    LinhaASM noOperator; // add zero, zero, zero
    noOperator.instrucao = "00000000000000000000000000110011";
    noOperator.isManualNOP = true;

    for (int i = instrucoes.size()-1; i >= 0; i--) {
        if (instrucoes[i].tipoInstrucao == "J") {
            instrucoes.insert(instrucoes.begin() + i, noOperator);
        }
    }
    return instrucoes;
}

// Solu��o 1
vector<LinhaASM> inserirNOPs(vector<LinhaASM> instrucoes, vector<int> hazards, bool forwardingImplementado) {
    // Certo, nos temos um array contendo as instru��es I (origens cujo possuem no minimo 1 hazard nas proximas 2 linhas)
    // Se formos de baixo pra cima, o array ser� mais f�cil de mexer
    // Ao verificar as instru��es, precisamos ir de cima pra baixo a partir de I, come�ando em I+1 at� ser maior que I+2
    // Se o hazard for na primeira linha, ser� necess�rio adicionar dois NOPs
    // Se o hazard for na segunda linha, ser� necess�rio adicionar um NOP

    LinhaASM noOperator; // add zero, zero, zero
    noOperator.instrucao = "00000000000000000000000000110011";

    noOperator.opcode = noOperator.instrucao.substr(25, 7);
    noOperator.rd = noOperator.instrucao.substr(20, 5);
    noOperator.funct3 = noOperator.instrucao.substr(17, 3);
    noOperator.rs1 = noOperator.instrucao.substr(12, 5);
    noOperator.rs2 = noOperator.instrucao.substr(7, 5);
    noOperator.funct7 = noOperator.instrucao.substr(0, 7);
    noOperator.tipoInstrucao = lerOpcode(noOperator.opcode);
    noOperator.isManualNOP = true;

    for (int i = hazards.size() - 1; i >= 0; i--) {
        // Quantidade de NOPs a serem adicionados, 2 se n�o tiver forwarding, 1 se tiver
        int quantNOPs = forwardingImplementado ? 1 : 2;

        for (int j = hazards[i] + 1; j <= hazards[i] + (forwardingImplementado ? 1 : 2); j++) {
            // Ignora itera��o j caso passe da quantidade de instru��es
            if (j > instrucoes.size() - 1) continue;

            if (verificarHazardInstrucao(instrucoes[hazards[i]], instrucoes[j], forwardingImplementado)) {
                for (int k = 0; k < quantNOPs; k++) {
                    instrucoes.insert(instrucoes.begin() + hazards[i] + 1, noOperator);
                }
            }
            quantNOPs--;
        }
    }
    cout << "NO OPERATORS INSERIDOS COM SUCESSO" << endl;
    return inserirNOPsEmJump(instrucoes);
}

vector<LinhaASM> aplicarReordenacao(vector<LinhaASM> instrucoes, vector<int> hazards, bool forwardingImplementado) {
    // Passa por todas as hazards
    for (int i = 0; i < hazards.size(); i++) {
        LinhaASM instrucaoEscolhida;
        bool instrucaoEscolhidaDefinida = false;
        int indiceInstrucaoEscolhida = 0;

        // Passa por todas as instru��es abaixo da linha da hazard
        for (int j = i+1; j < instrucoes.size(); j++) {

            // Valida��o da linha p/ entrar em hazard+1 //

            // Passa linha j se ela conflita com a linha da hazard
            if (verificarHazardInstrucao(instrucoes[hazards[i]], instrucoes[j], forwardingImplementado)) continue;

            // Passa linha j se n�o tiver forwarding e ela conflitar com hazard-1
            if (!forwardingImplementado && i > 0){
                if (verificarHazardInstrucao(instrucoes[hazards[i] - 1], instrucoes[j], forwardingImplementado)) continue;
            }

            bool linhaValidaDepois = true;
            // Passa linha j se as linhas ap�s a hazard n�o ter�o conflito com a linha j
            for (int k = hazards[i] + 1; k <= hazards[i] + (forwardingImplementado ? 1 : 2); k++) {
                // Evita k de atravessar o tamanho maximo do vetor
                if (k > instrucoes.size() - 1) continue;
                if (verificarHazardInstrucao(instrucoes[k], instrucoes[j], forwardingImplementado)) linhaValidaDepois = false;
            }

            // Valida��o da linha ao sair de seu ponto de origem //

            bool linhaValidaAntes = true;
            // Passa linha j se a linha j-1 n�o tiver conflito com as proximas linhas caso a linha j seja removida
            for (int k = j + 1; k <= j + (forwardingImplementado ? 1 : 2); k++) {
                // Evita k de atravessar o tamanho maximo do vetor
                if (k > instrucoes.size() - 1) continue;
                if (verificarHazardInstrucao(instrucoes[j - 1], instrucoes[k], forwardingImplementado)) linhaValidaAntes = false;
            }

            if (linhaValidaAntes && linhaValidaDepois) {
                instrucaoEscolhida = instrucoes[j];
                instrucaoEscolhidaDefinida = true;
                indiceInstrucaoEscolhida = j;
            }
        }

        // H� uma instru��o que pode ser reordenada
        if (instrucaoEscolhidaDefinida) {
            instrucoes.insert(instrucoes.begin() + hazards[i] + 1, instrucaoEscolhida);
            instrucoes.erase(instrucoes.begin() + indiceInstrucaoEscolhida + 1);
        }
    }
    return instrucoes;
}


// Verifica o vetor de instru��es assembly por
// hazards de pipeline
// regra base: rd atual n�o pode ser utilizado
// nos proximos 2 ciclos
// exceto caso haja implementa��o de fowarding, que ai
// � apenas um ciclo
vector<int> verificarHazards(vector<LinhaASM> instrucoes, bool forwardingImplementado) {
    cout << "Executando verificacao de hazards" << endl;
    vector<int> falhas;
    for (int i = 0; i < instrucoes.size(); i++) {
        // Ignora instru��o se o registrador de destino for zero, NOP
        if (instrucoes[i].tipoInstrucao != "S") {
            if (instrucoes[i].rd == "00000") continue;
        }

        // For loop 2 passos a frente de i
        // verifica��o de dependencias
        for (int j = i+1; j <= i+2; j++) {
            // Ignora itera��o j caso passe da quantidade de instru��es
            if (j >= instrucoes.size()) continue;

            // Ignora segunda itera��o caso forwarding esteja implementado
            if (j == i + 2 && forwardingImplementado) continue;

            if (verificarHazardInstrucao(instrucoes[i], instrucoes[j], forwardingImplementado)) {
                cout << "| Hazard encontrada na linha " << j + 1 << " vindo da linha nao-finalizada " << i + 1 << endl;
                falhas.push_back(i);
            }
        }
    }

    if (falhas.size() == 0) cout << "Nenhum hazard com encontrado" << endl;
    cout << "Verificacao de hazards concluido com exito" << endl;
    return falhas;
}

// Recebe um ifstream para ler e gera um vetor
// cada indice do vetor representa uma instru��o de 32 bits
vector<LinhaASM> lerArquivo(ifstream& arquivo) {
    vector<LinhaASM> instrucoes;

    if (!arquivo.is_open()) {
        throw std::runtime_error("Abra o arquivo antes de tentar ler!");
    }

    int i = 0;
    while (arquivo.good()) {
        i++;
        LinhaASM linhaAtual;
        arquivo >> linhaAtual.instrucao;

        if (linhaAtual.instrucao.size() != 32) {
            if (arquivo.good()) {
                // alguma linha n�o est� certa
                throw std::runtime_error("Nao foi possivel ler a linha " + to_string(i) + ", verifique o arquivo.");
            }
            else {
                // newline final, s� ignorar
                continue;
            }
        }

        // obs: a ordem dos numeros s�o o inverso do site
        // https://jemu.oscc.cc/ADD
        // (site vai da direita pra esquerda, aqui � esq. pra dir.)
        linhaAtual.opcode = linhaAtual.instrucao.substr(25, 7);
        linhaAtual.rd = linhaAtual.instrucao.substr(20, 5);
        linhaAtual.funct3 = linhaAtual.instrucao.substr(17, 3);
        linhaAtual.rs1 = linhaAtual.instrucao.substr(12, 5);
        linhaAtual.rs2 = linhaAtual.instrucao.substr(7, 5);
        linhaAtual.funct7 = linhaAtual.instrucao.substr(0, 7);

        linhaAtual.tipoInstrucao = lerOpcode(linhaAtual.opcode);
        // se tudo deu certo, pode colocar no array
        instrucoes.push_back(linhaAtual);
    }


    return instrucoes;
}

// Retorna uma organizacao conforme
// entrada do usu�rio
Organizacao criarOrganizacao(string nome) {
    cout << "----------------------\n ORGANIZACAO " << nome << endl;
    Organizacao resultado;
    cout << "Forneca o tempo de clock da organizacao " << nome << ": ";
    cin >> resultado.TClock;
    resultado.freqClock = (1 / resultado.TClock);
    cout << "----------------------" << endl << endl;
    return resultado;
}

Resultados calcularResultados(vector<LinhaASM> programa, Organizacao organizacao) {
    Resultados resultado{};

    for (int i = 0; i < programa.size(); i++) {
        resultado.CiclosTotais += (i == 0 ? 5 : 1);
    }

    resultado.CPI = gerarCPI(resultado.CiclosTotais, programa.size());
    resultado.TExec = TExecCPUPorTempoClock(programa.size(), resultado.CPI, organizacao.TClock);
    resultado.desempenho = gerarDesempenho(resultado.TExec);
    return resultado;
}


vector<LinhaASM> solucao(int tecnica, string nomeFornecido) {
    cout << endl << endl << endl;
    cout << "Executando solucao " << tecnica << endl;
    ifstream programa;
    abrirArquivo(programa, nomeFornecido);
    vector<LinhaASM> instrucoes = lerArquivo(programa);
    cout << "Instrucoes originais: " << endl;
    VisualizarInstrucoes(instrucoes);
    vector<int> falhas;
    bool forwardingImplementado;

    switch (tecnica)
    {
        case 1:
            forwardingImplementado = false;
            falhas = verificarHazards(instrucoes, forwardingImplementado);
            instrucoes = inserirNOPs(instrucoes, falhas, forwardingImplementado);
            salvarPrograma(instrucoes, "Solucao1_NOP-NoForward.txt");
            break;

        case 2:
            forwardingImplementado = true;
            falhas = verificarHazards(instrucoes, forwardingImplementado);
            instrucoes = inserirNOPs(instrucoes, falhas, forwardingImplementado);
            salvarPrograma(instrucoes, "Solucao2_NOP-Forward.txt");
            break;

        case 3: // sem hardware, reordenamento, nops
            forwardingImplementado = false;
            falhas = verificarHazards(instrucoes, forwardingImplementado);
            instrucoes = aplicarReordenacao(instrucoes, falhas, forwardingImplementado);
            falhas = verificarHazards(instrucoes, forwardingImplementado);
            instrucoes = inserirNOPs(instrucoes, falhas, forwardingImplementado);
            salvarPrograma(instrucoes, "Solucao3_Reordenamento-NOP-NoForward.txt");
            break;

        case 4: // forwarding, reordenamento, nops
            forwardingImplementado = true;
            falhas = verificarHazards(instrucoes, forwardingImplementado);
            instrucoes = aplicarReordenacao(instrucoes, falhas, forwardingImplementado);
            falhas = verificarHazards(instrucoes, forwardingImplementado);
            instrucoes = inserirNOPs(instrucoes, falhas, forwardingImplementado);
            salvarPrograma(instrucoes, "Solucao4_Reordenamento-NOP-Forward.txt");
            break;

        default:
            cout << "[AVISO] Solucao " << tecnica << " nao implementada!" << endl;
            vector<LinhaASM> respostaVazia;
            return respostaVazia;
    }

    cout << "Instrucoes com a solucao " << tecnica << ":" << endl;
    VisualizarInstrucoes(instrucoes);
    cout << "Verificacao de hazards pos-modificacao:" << endl;
    verificarHazards(instrucoes, forwardingImplementado);
    return instrucoes;
}

int main() {
    bool finalizado = false;
    bool semOrganizacao = true;
    Organizacao orgA;
    Organizacao orgB;
    while (!finalizado) {
        system("cls");

        // Cria as organiza��es se elas n�o existem ou usuario quer refazer
        if (semOrganizacao) {
            cout << "Criando organizacoes: " << endl;
            semOrganizacao = false;
            orgA = criarOrganizacao("A");
        }

        string nomeFornecido = "";
        cout << "Forneca o nome/caminho do arquivo contendo as instrucoes: " << endl;
        cin >> nomeFornecido;
        cout << endl << endl;
        int tecnica = 0;

        do {
            cout << "Escolha a tecnica:" << endl;
            cout << "1 - Sem solucao em hardware, insercao de NOPs" << endl;
            cout << "2 - Forwarding, insercao de NOPs" << endl;
            cout << "3 - Sem solucao em hardware, reordenamento, insercao de NOPs" << endl;
            cout << "4 - Forwarding. reordenamento, insercao de NOPs" << endl;
            cout << "5 - Todas as opcoes acima" << endl;
            cin >> tecnica;

            if (tecnica < 1 || tecnica > 5) {
                cout << "Opcao invalida, tente novamente!" << endl;
            }
        } while (tecnica < 1 || tecnica > 5);

        for (int i = 1; i <= 4; i++) {
            if (tecnica != 5 && i != tecnica) continue;
            vector<LinhaASM> instrucaoResolvida = solucao(i, nomeFornecido);
            if (!instrucaoResolvida.empty()) {
                cout << endl;
                cout << "Resultados da solucao " << i << ":" << endl;
                Resultados resultado = calcularResultados(instrucaoResolvida, orgA);
                cout << "Total de ciclos: " << resultado.CiclosTotais << endl;
                cout << "CPI (Ciclos por Instrucao): " << resultado.CPI << endl;
                cout << "Tempo de execucao: " << resultado.TExec << endl;
                cout << "Desempenho: " << resultado.desempenho << endl;
                contarNOPs(instrucaoResolvida);
            }
        }



        /*
        * Debug das informa��es gerais
        for (int i = 0; i < instrucoes.size(); i++) {
            cout << "INSTRUCAO " << i + 1 << " completa: " << instrucoes[i].instrucao << endl;
            cout << "TIPO DE INSTRUCAO: " << instrucoes[i].tipoInstrucao << endl;
            cout << "INSTRUCAO " << i + 1 << " separada: "
                << instrucoes[i].funct7 << " "
                << instrucoes[i].rs2 << " "
                << instrucoes[i].rs1 << " "
                << instrucoes[i].funct3 << " "
                << instrucoes[i].rd << " "
                << instrucoes[i].opcode
                << endl << endl;
        }
        */

        /*
        Resultados resultadoA = calcularResultados(instrucoes, orgA);
        cout << "RESULTADOS DA ORGANIZACAO A: " << endl;
        cout << "TOTAL DE CICLOS: " << resultadoA.CiclosTotais << endl;
        cout << "CPI (Ciclos por Instrucao): " << resultadoA.CPI << endl;
        cout << "Tempo de execucao: " << resultadoA.TExec << endl;

        cout << "----------------------" << endl;
        */


        int escolha = 0;
        cout << endl << "Escolha uma opcao:" << endl;
        cout << "1 - Abrir novo arquivo" << endl;
        cout << "2 - Recriar organizacoes e abrir novo arquivo" << endl;
        cout << "3 - Sair" << endl;
        while (escolha < 1 || escolha > 3) {
            cout << "Opcao escolhida: ";
            cin >> escolha;
            if (escolha < 1 || escolha > 3)
                cout << "Opcao invalida, tente novamente!" << endl;
        }
        cout << endl << endl;

        switch (escolha)
        {
        case 2:
            semOrganizacao = true;
            break;

        case 3:
            finalizado = true;
            break;
        }
    }
    return 0;
}
