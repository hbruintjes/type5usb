# get status
add_custom_target(
	get_status
	${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_OPTIONS} -p ${AVR_MCU} -c ${AVR_PROGRAMMER} -P ${AVR_UPLOADTOOL_PORT} -n -v
	COMMENT "Get status from ${AVR_MCU}"
)

# get fuses
add_custom_target(
	get_fuses
	${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_OPTIONS} -p ${AVR_MCU} -c ${AVR_PROGRAMMER} -P ${AVR_UPLOADTOOL_PORT} -n
		-U lfuse:r:-:b
		-U hfuse:r:-:b
	COMMENT "Get fuses from ${AVR_MCU}"
)

# set fuses
add_custom_target(
	set_fuses
	${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_OPTIONS} -p ${AVR_MCU} -c ${AVR_PROGRAMMER} -P ${AVR_UPLOADTOOL_PORT}
		-U lfuse:w:${AVR_L_FUSE}:m
		-U hfuse:w:${AVR_H_FUSE}:m
		COMMENT "Setup: High Fuse: ${AVR_H_FUSE} Low Fuse: ${AVR_L_FUSE}"
)

# get oscillator calibration
add_custom_target(
	get_calibration
		${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_OPTIONS} -p ${AVR_MCU} -c ${AVR_PROGRAMMER} -P ${AVR_UPLOADTOOL_PORT}
		-U calibration:r:${AVR_MCU}_calib.tmp:r
		COMMENT "Write calibration status of internal oscillator to ${AVR_MCU}_calib.tmp."
)

# set oscillator calibration
add_custom_target(
	set_calibration
	${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_OPTIONS} -p ${AVR_MCU} -c ${AVR_PROGRAMMER} -P ${AVR_UPLOADTOOL_PORT}
		-U calibration:w:${AVR_MCU}_calib.hex
		COMMENT "Program calibration status of internal oscillator from ${AVR_MCU}_calib.hex."
)
