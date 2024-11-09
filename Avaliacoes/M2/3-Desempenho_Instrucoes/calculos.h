/*
* Disciplina: Organização de Computadores
* Atividade : Avaliação 02
*
* Grupo:
* - Cassiano de Sena Crispim
* - Hérick Vitor Vieira Bittencourt
* - Eduardo Miguel Fuchs Perez
*
* OBSERVAÇÃO:
* - Devido o uso de for loops com lógica [key, value], o padrão ISO para
* este projeto deve ser o C++ 17, caso contrário a build irá falhar
* (vá na aba projeto->propriedades de main no visual studio 2019)
*/

#ifndef CALCULOS_H
#define CALCULOS_H

#include <iostream>
#include <fstream>

using namespace std;

// Calculos feitos a partir da tabela de formulas disponivel no AVA

double gerarDesempenho(float tempoExecucao) {
	return 1.0 / tempoExecucao;
}

// Tempo de execução da CPU por tempo de clock
// Formula = QuantInstrucoes * CPI * TempoClock
// OBS: CiclosCPU = QuantInstrucoes * CPI
float TExecCPUPorTempoClock(int quantInstrucoes, float CPI, float tempoClock) {
	return quantInstrucoes * CPI * tempoClock;
}

// Ciclos-Por-Instrução
// Formula = (5 [primeira instrução] + (1 * (quantInstrucoes-1) [1 clock p/ instruções seguintes])) / quantInstrucoes
float gerarCPI(float ciclosCPU, int quantInstrucoes) {
	return (5.0 + (1 * (quantInstrucoes-1))) / quantInstrucoes;
}

#endif // CALCULOS_H