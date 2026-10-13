import os

def generate_adc_lut_header(output_dir=".", filename="ADC_Array_LUT.h", bits=12, vref=3.3, r_top=100_000.0, r_bottom=2490.0, amp_gain=0.4, array_name="Array_LUT"):
    """
    Generates a C/C++ header file containing a lookup table that maps raw ADC 
    counts directly to millivolts (mV), factoring in attenuation and amplification.
    
    Parameters:
    - output_dir (str): The directory where the header file should be saved.
    - filename (str): The name of the output header file.
    - array_name (str): The name of the LUT array in the generated header.
    """
    max_counts = (2 ** bits) - 1
    divider_ratio = r_bottom / (r_top + r_bottom)
    
    lut_entries = []
    
    # Calculate millivolts for every possible raw ADC count
    for counts in range(2 ** bits):
        # 1. Voltage at ADC pin
        v_adc_pin = (counts / max_counts) * vref
        # 2. Revert amplifier gain
        v_divider_out = v_adc_pin / amp_gain
        # 3. Revert resistor divider
        v_original_volts = v_divider_out / divider_ratio
        # 4. Convert to mV and round to closest integer
        v_original_mv = round(v_original_volts * 1000)
        
        lut_entries.append(int(v_original_mv))
        
    # Capture the max voltage string for the comment header
    max_voltage_mv = lut_entries[-1]

    # Construct the file layout
    header_content = f"""#pragma once

#include <stdint.h>

// {bits}-bit ADC Lookup Table
// Output Units: millivolts (mV)
// Range: 0 mV to {max_voltage_mv} mV

static const uint32_t {array_name}[{2 ** bits}] = {{
"""

    # Format the data cleanly with 8 entries per row
    entries_per_line = 8
    for i in range(0, len(lut_entries), entries_per_line):
        line_chunk = lut_entries[i : i + entries_per_line]
        line_str = ", ".join(str(val) for val in line_chunk)
        
        # Append commas except for the very last element chunk
        if i + entries_per_line < len(lut_entries):
            header_content += f"    {line_str},\n"
        else:
            header_content += f"    {line_str}\n"

    header_content += "};\n"

    # Make output_dir relative to the script file location, not cwd
    script_dir = os.path.dirname(os.path.abspath(__file__))
    if output_dir and not os.path.isabs(output_dir):
        output_dir = os.path.join(script_dir, output_dir)

    # Ensure the target directory exists before writing
    if output_dir and output_dir != ".":
        os.makedirs(output_dir, exist_ok=True)
        
    full_path = os.path.join(output_dir, filename)

    # Write out the file
    with open(full_path, "w") as f:
        f.write(header_content)
        
    print(f"Successfully generated header: '{full_path}'")
    print(f"LUT Range: 0 mV to {max_voltage_mv} mV")


if __name__ == "__main__":
    # Specify your desired output folder path here (e.g., your project's include directory)
    TARGET_DIR = "../Drivers/Inc" 
    
    generate_adc_lut_header(
        output_dir=TARGET_DIR,
        filename="ADC_Array_LUT.h",
        bits=12,
        vref=3.029,
        r_top=100000.0,   
        r_bottom=2490.0,  
        amp_gain=0.4,
        array_name="Array_LUT"
    )

    generate_adc_lut_header(
        output_dir=TARGET_DIR,
        filename="ADC_Battery_LUT.h",
        bits=12,
        vref=3.029,
        r_top=100000.0,   
        r_bottom=2490.0,  
        amp_gain=0.4,
        array_name="Battery_LUT"
    )