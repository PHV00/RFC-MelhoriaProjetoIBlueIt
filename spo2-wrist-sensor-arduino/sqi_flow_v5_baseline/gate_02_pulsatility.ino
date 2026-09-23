/*
 * GATE 02 - PULSATILIDADE
 * =======================
 *
 * STATUS: DOCUMENTADO, AINDA NAO IMPLEMENTADO.
 * O codigo deste Gate so sera escrito depois da validacao experimental do G1.
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
 * - amplitude / dynamic range;
 * - threshold crossings;
 * - autocorrelacao (ACF);
 * - decisao hierarquica em janela curta.
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
 *   PPG pre-processado
 *          |
 *          +--> amplitude plausivel? ----+
 *          |                              |
 *          +--> crossings plausiveis? ---+--> estrutura periodica? --> PASS
 *          |                              |                         |
 *          +--> ACF com pico util? -------+                         +--> FAIL
 *                                                                         |
 *                                                                         v
 *                                                                      INVALID
 *
 * RISCO CONHECIDO
 * ---------------
 * Movimento periodico tambem pode produzir autocorrelacao alta. Por isso G2
 * nao encerra a avaliacao: G3 verifica a morfologia dos pulsos.
 */

// TODO: implementar somente apos a validacao do Gate 01.
