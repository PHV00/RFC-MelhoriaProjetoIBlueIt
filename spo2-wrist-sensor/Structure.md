[SENSOR MAX3010x]
       |
       v
[drivers/]
  le hardware bruto
       |
       v
[sensing/]
  amostra + buffer + contato
       |
       v
[processing/]
  filtro + qualidade + FC + SpO2
       |
       v
[safety/]
  confiança + limiares + fail-safe
       |
       +------------------+
       |                  |
       v                  v
[transport/]          [storage/]
 BLE / Serial         config / calibração / logs
       |
       v
[app/] 
 orquestra estados e decisões do sistema