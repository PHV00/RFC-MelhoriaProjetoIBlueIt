/*
 * GATE 03 - MORFOLOGIA LEVE
 * =========================
 *
 * STATUS: DOCUMENTADO, AINDA NAO IMPLEMENTADO.
 * O codigo deste Gate so sera escrito depois de G1 e G2 serem validados.
 *
 * PERGUNTA DO GATE
 * ----------------
 * "Os pulsos detectados apresentam forma e estabilidade beat-to-beat
 *  compativeis com um PPG utilizavel?"
 *
 * FUNDAMENTACAO PRINCIPAL
 * -----------------------
 * Sukor et al. - Signal quality measures for pulse oximetry through waveform
 * morphology analysis.
 * Ano: 2011
 * DOI: 10.1088/0967-3334/32/3/008
 *
 * Elementos previstos para a V1:
 * - pulse amplitude;
 * - pulse width;
 * - estabilidade/similaridade entre batimentos.
 *
 * COMPLEMENTO
 * -----------
 * Fischer et al. - An Algorithm for Real-Time Pulse Waveform Segmentation
 * and Artifact Detection in Photoplethysmograms.
 * DOI: 10.1109/JBHI.2016.2518202
 *
 * Elementos complementares previstos:
 * - rise time;
 * - pulse duration;
 * - comparacao beat-to-beat.
 *
 * O QUE VAMOS VALIDAR
 * -------------------
 *
 *     /¯\      /¯\      /¯\
 *    /   \    /   \    /   \
 * __/     \__/     \__/     \__
 *    |<-->|      comparar batimentos
 *    width
 *      ^
 *      +-- amplitude / subida / estabilidade
 *
 *   pulsos estaveis ----------------------------> PASS
 *   ruido periodico com forma inconsistente ----> FAIL -> INVALID
 *
 * ESCOPO V1
 * ---------
 * Nao implementar inicialmente template complexo, alinhamento completo ou
 * distancia Euclidiana. Esses metodos ficam reservados para uma V2 caso a
 * morphology-lite seja insuficiente.
 */

// TODO: implementar depois da validacao dos Gates 01 e 02.
