/*
 * GATE 02 - PULSATILIDADE
 * =======================
 *
 * STATUS: ARQUITETURA DE V1 DEFINIDA, AINDA NAO IMPLEMENTADO.
 * O Gate 01 e o armazenamento Packed18 ja foram validados em bancada.
 * Este arquivo agora documenta o contrato e o plano experimental do G2 antes
 * da escrita do algoritmo.
 *
 * PERGUNTA DO GATE
 * ----------------
 * "A janela contem uma estrutura pulsatile/periodica plausivel, em vez de
 *  apenas variacao aleatoria ou artefato?"
 *
 * FUNDAMENTACAO PRINCIPAL
 * -----------------------
 * Vadrevu et al. - Real-Time PPG Signal Quality Assessment System for
 * Improving Battery Life and False Alarms.
 * Ano: 2019
 * DOI: 10.1109/TCSII.2019.2891636
 *
 * Elementos previstos para a V1:
 * - remover o nivel DC (centralizacao pela media da janela);
 * - medir amplitude AC relativa ao nivel DC;
 * - localizar estrutura repetitiva/periodica por autocorrelacao (ACF);
 * - procurar o melhor lag dentro de uma faixa fisiologicamente plausivel;
 * - exigir evidencia consistente nos dois canais, sem transformar G2 em G4.
 *
 * A amplitude/range absoluto ja e verificada grosseiramente pelo G1. No G2,
 * o interesse passa a ser a componente pulsatile e sua periodicidade.
 *
 * IMPORTANTE
 * ----------
 * Os thresholds do artigo NAO serao copiados diretamente. A planilha de
 * arquitetura do projeto define esses parametros como configuraveis e sujeitos
 * a caracterizacao no MAX30102.
 *
 * O QUE VAMOS VALIDAR
 * -------------------
 *
 *   janela aprovada no G1 (Packed18)
 *          |
 *          v
 *   reconstruir amostra em uint32_t sob demanda
 *          |
 *          +--> remover DC / estimar AC -------+
 *          |                                    |
 *          +--> AC/DC minimamente plausivel? ---+--> pulsatividade? --> PASS
 *          |                                    |                  |
 *          +--> ACF com pico util? -------------+                  +--> FAIL
 *                                                                         |
 *                                                                         v
 *                                                                      INVALID
 *
 * RISCO CONHECIDO
 * ---------------
 * Movimento periodico tambem pode produzir autocorrelacao alta. Por isso G2
 * nao encerra a avaliacao: G3 verifica a morfologia dos pulsos.
 */

/*
 * CONTRATO DE MEMORIA PARA A IMPLEMENTACAO
 * ----------------------------------------
 * - reutilizar o buffer Packed18 ja existente;
 * - NAO criar dois novos arrays uint32_t[100];
 * - reconstruir uint32_t apenas quando a amostra for usada;
 * - priorizar acumuladores/indices pequenos;
 * - qualquer workspace adicional deve ser medido com freeRAM().
 *
 * PLANO DE VALIDACAO
 * ------------------
 * A) dedo estavel: deve apresentar estrutura periodica repetivel;
 * B) sem dedo: nem chega ao G2 porque deve falhar no G1;
 * C) colocacao/retirada do dedo: G1 pode passar, mas G2 deve tender a rejeitar;
 * D) movimento proposital: verificar falsos positivos por periodicidade de movimento;
 * E) pressao excessiva/contato ruim: observar reducao de AC/DC e estabilidade da ACF.
 *
 * Nenhum threshold numerico do G2 esta fechado neste momento. Primeiro sera
 * implementada instrumentacao que imprima as metricas; os limites serao
 * definidos depois da caracterizacao experimental no nosso MAX30102.
 */

// TODO V1: implementar metricas e modo diagnostico antes de habilitar decisao PASS/FAIL.
