SYSTEM
|
+-- AUTHORING_LAYER  (structure is authored)
|   |
|   +-- circuit_model
|   |   +-- components
|   |   +-- wires
|   |   +-- nodes
|   |   +-- ports
|   |   `-- hierarchy
|   |
|   +-- edit_system
|   |   +-- create
|   |   +-- delete
|   |   +-- connect
|   |   +-- duplicate
|   |   `-- undo_redo
|   |
|   +-- ui_semantics
|   |   +-- hit_testing
|   |   +-- placement_rules
|   |   +-- connection_rules
|   |   `-- viewport
|   |
|   `-- export_to_compiler
|
+-- STRUCTURAL_COMPILER  (structure -> function translation)
|   |
|   +-- import_normalization
|   |   +-- bench_loader
|   |   +-- blif_loader
|   |   `-- graph_normalizer
|   |
|   +-- structural_validation
|   |   +-- width_checks
|   |   +-- port_checks
|   |   +-- fanout_checks
|   |   `-- state_contract_checks
|   |
|   +-- recognition
|   |   +-- motif_detection
|   |   +-- arithmetic_patterns
|   |   +-- control_patterns
|   |   `-- sequential_patterns
|   |
|   +-- macro_substitution
|   |   +-- mux
|   |   +-- adders
|   |   +-- parity
|   |   +-- register_blocks
|   |   `-- alu_patterns
|   |
|   `-- plan_generation
|       +-- levelization
|       +-- scheduling
|       +-- chunk_plan
|       +-- macro_plan
|       `-- functional_region_plan
|
+-- ENGINE  (function execution)
|   |
|   +-- runtime_state_model
|   |   |
|   |   +-- net_banks
|   |   |   +-- net_value[]
|   |   |   `-- net_mask[]
|   |   |
|   |   +-- sequential_banks
|   |   |   +-- current_state
|   |   |   `-- next_state
|   |   |
|   |   +-- io_banks
|   |   |   +-- inputs
|   |   |   `-- outputs
|   |   |
|   |   `-- probe_banks
|   |
|   +-- execution_scheduler
|   |   |
|   |   +-- plan_walker
|   |   +-- chunk_dispatcher
|   |   +-- macro_dispatcher
|   |   +-- functional_region_dispatcher
|   |   `-- sequential_commit_path
|   |
|   +-- native_function_catalog
|   |   |
|   |   +-- primitive_functions
|   |   |   +-- AND
|   |   |   +-- OR
|   |   |   +-- XOR
|   |   |   +-- NOT
|   |   |   +-- BUF
|   |   |   `-- TRI
|   |   |
|   |   +-- standard_macros
|   |   |   +-- MUX
|   |   |   +-- XOR2
|   |   |   +-- XNOR2
|   |   |   `-- PARITY
|   |   |
|   |   +-- multi_macros
|   |   |   +-- HALF_ADDER
|   |   |   +-- FULL_ADDER
|   |   |   +-- RIPPLE_ADD
|   |   |   `-- CARRY_SAVE
|   |   |
|   |   `-- stateful_functions
|   |       +-- registers
|   |       +-- ROM
|   |       +-- RAM
|   |       +-- regfile
|   |       `-- ALU
|   |
|   +-- storage_function_executors
|   |   +-- rom_executor
|   |   +-- ram_executor
|   |   `-- regfile_executor
|   |
|   +-- diagnostics_surface
|   |   |
|   |   +-- runtime_counters
|   |   +-- state_readers
|   |   `-- structural_metadata
|   |
|   `-- runtime_api
|       |
|       +-- plan_intake
|       +-- session_management
|       +-- state_queries
|       `-- diagnostics_queries
|
`-- TOOLING_LAYER
    |
    +-- blif2bench
    +-- corpus_tools
    +-- benchmark_packaging
    `-- report_tools
