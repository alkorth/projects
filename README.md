# projects
ETW traces collection and post processing
Goal: demonstrate capabilities to collect features for malware detection from ETW traces on Windows

The project contains 3 utilities:
- TraceCollector utility
  The utility executes given executable until it terminates or preset timeout expires, the utility collects ETW traces (preconfigured by GUIDs and system logger traces)
  during the execution in binary (ETL) format, saves collected traces together with information about executed process (PID)
- CsvParser utility
  The utility works with data collected by the TraceCollector utility: converts the data to CSV file format (with help of WDK utility tracerpt),
  converted data parsed again to extract features (with help of PID of executed process) that can be used to train ML model
- CycleExec utility
  The helper utility used to automate process of collection of traces information from multiple samples(executables).
  Important work assumptions:
  - Preconfigured Hyper-V VM exists, the VM configured to execute a batch file upon start
  - The utility executes pre-created several power shell scripts for next goals:
    = Start VM
    = Stop VM
    = Mount VM HDD
    = Dismount VM HDD
    = Revert VM to original state
    
  Each iteration of the utility (per sample to be executed) performs next:
  - Mount VM HDD
  - Copy the sample to the mounted HDD
  - Modify startup execution batch file to execute the sample via TraceCollector utility
  - Dismount the VM HDD
  - Start the VM
  - Wait for preconfigured period of time
  - Stop the VM
  - Mount the VM HDD
  - Retrieve collected execution traces
  - Dismount the VM HDD
  - Revert the VM to original state
  Such set of operations required to ensure that on each iteration given VM won't be affected by earlier iterations, also those measures ensures that if given sample 
  represent malware, it's execution will be done in controlled isolated environment which will be disposed right after execution
  
