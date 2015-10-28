emonTH_gas_reflection_analogue - Enables the emonTH as a gas meter node using a phototransistor (or other pulse calculated from analogue input). Note that you will need to experiment to get the best position for your sensing apparatus (recommend an IR LED and matched phototransistor) and configure the sketch accordingly. Average and lowest readings are reported as extra inputs to help with calibration through emonCMS. While power requirements are significantly higher than for temperature monitoring (or interrupt-based pulse monitoring) these should still be respectable; tests are ongoing to determine battery life.


emonTH_PulseCounting Old - now merged into Optical (or wired) pulse counting example of interfacing with pulse-output utility meters
