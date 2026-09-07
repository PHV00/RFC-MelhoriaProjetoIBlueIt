set(SQI_G1_FAULT_MODE "0" CACHE STRING "G1 validation fault mode: 0=off, 1=flatline, 2=clip-red, 3=clip-ir, 4=discontinuity")
set_property(CACHE SQI_G1_FAULT_MODE PROPERTY STRINGS 0 1 2 3 4)
message(STATUS "SQI G1 fault injection mode: ${SQI_G1_FAULT_MODE}")

idf_component_register(
    SRCS
        "main.c"
        "app/app_controller.c"
        "app/app_state_machine.c"
        "drivers/i2c_bus.c"
        "drivers/max3010x_driver.c"
        "sensing/sample_buffer.c"
        "sensing/ppg_sampler.c"
        "processing/sqi/signal_quality.c"
        "processing/sqi/gates/g1_integrity/gate_integrity.c"
        "processing/hr_estimator.c"
        "processing/spo2_estimator.c"
        "safety/confidence_engine.c"
        "transport/serial_telemetry.c"
        "storage/config_repo.c"
    INCLUDE_DIRS
        "."
    REQUIRES
        driver
        esp_timer
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE SQI_G1_FAULT_MODE=${SQI_G1_FAULT_MODE})
