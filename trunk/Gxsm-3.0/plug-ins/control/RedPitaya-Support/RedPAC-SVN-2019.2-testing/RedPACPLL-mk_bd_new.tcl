
################################################################
# This is a generated script based on design: system
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
################################################################

namespace eval _tcl {
proc get_script_folder {} {
   set script_path [file normalize [info script]]
   set script_folder [file dirname $script_path]
   return $script_folder
}
}
variable script_folder
set script_folder [_tcl::get_script_folder]

################################################################
# Check if script is running in correct Vivado version.
################################################################
set scripts_vivado_version 2019.2
set current_vivado_version [version -short]

if { [string first $scripts_vivado_version $current_vivado_version] == -1 } {
   puts ""
   catch {common::send_msg_id "BD_TCL-109" "ERROR" "This script was generated using Vivado <$scripts_vivado_version> and is being run in <$current_vivado_version> of Vivado. Please run the script in Vivado <$scripts_vivado_version> then open the design in Vivado <$current_vivado_version>. Upgrade the design by running \"Tools => Report => Report IP Status...\", then run write_bd_tcl to create an updated script."}

   return 1
}

################################################################
# START
################################################################

# To test this script, run the following commands from Vivado Tcl console:
# source system_script.tcl


# The design that will be created by this Tcl script contains the following 
# module references:
# VolumeAdjuster16_14, axis_dc_filter, axis_decimator, axis_sc28_to_14, cfg_select, cfg_select, cfg_select, cfg_select, cfg_select, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_to_axis, led_connect, lms_phase_amplitude_detector, controller_pi, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_select, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_to_axis, logic_or, controller_pi, phase_unwrap, McBSP_controller, McBSP_io_connect, ScaleAndAdjust, axis_4s_combine, axis_bram_push, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_to_axis, cfg_to_axis

# Please add the sources of those modules before sourcing this Tcl script.

# If there is no project opened, this script will create a
# project, but make sure you do not have an existing project
# <./myproj/project_1.xpr> in the current working folder.

set list_projs [get_projects -quiet]
if { $list_projs eq "" } {
   create_project project_1 myproj -part xc7z010clg400-1
}


# CHANGE DESIGN NAME HERE
variable design_name
set design_name system

# If you do not already have an existing IP Integrator design open,
# you can create a design using the following command:
#    create_bd_design $design_name

# Creating design if needed
set errMsg ""
set nRet 0

set cur_design [current_bd_design -quiet]
set list_cells [get_bd_cells -quiet]

if { ${design_name} eq "" } {
   # USE CASES:
   #    1) Design_name not set

   set errMsg "Please set the variable <design_name> to a non-empty value."
   set nRet 1

} elseif { ${cur_design} ne "" && ${list_cells} eq "" } {
   # USE CASES:
   #    2): Current design opened AND is empty AND names same.
   #    3): Current design opened AND is empty AND names diff; design_name NOT in project.
   #    4): Current design opened AND is empty AND names diff; design_name exists in project.

   if { $cur_design ne $design_name } {
      common::send_msg_id "BD_TCL-001" "INFO" "Changing value of <design_name> from <$design_name> to <$cur_design> since current design is empty."
      set design_name [get_property NAME $cur_design]
   }
   common::send_msg_id "BD_TCL-002" "INFO" "Constructing design in IPI design <$cur_design>..."

} elseif { ${cur_design} ne "" && $list_cells ne "" && $cur_design eq $design_name } {
   # USE CASES:
   #    5) Current design opened AND has components AND same names.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 1
} elseif { [get_files -quiet ${design_name}.bd] ne "" } {
   # USE CASES: 
   #    6) Current opened design, has components, but diff names, design_name exists in project.
   #    7) No opened design, design_name exists in project.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 2

} else {
   # USE CASES:
   #    8) No opened design, design_name not in project.
   #    9) Current opened design, has components, but diff names, design_name not in project.

   common::send_msg_id "BD_TCL-003" "INFO" "Currently there is no design <$design_name> in project, so creating one..."

   create_bd_design $design_name

   common::send_msg_id "BD_TCL-004" "INFO" "Making design <$design_name> as current_bd_design."
   current_bd_design $design_name

}

common::send_msg_id "BD_TCL-005" "INFO" "Currently the variable <design_name> is equal to \"$design_name\"."

if { $nRet != 0 } {
   catch {common::send_msg_id "BD_TCL-114" "ERROR" $errMsg}
   return $nRet
}

set bCheckIPsPassed 1
##################################################################
# CHECK IPs
##################################################################
set bCheckIPs 1
if { $bCheckIPs == 1 } {
   set list_check_ips "\ 
pavel-demin:user:axis_red_pitaya_adc:1.0\
pavel-demin:user:axis_red_pitaya_dac:1.0\
xilinx.com:ip:clk_wiz:6.0\
xilinx.com:ip:cordic:6.0\
xilinx.com:ip:dds_compiler:6.0\
xilinx.com:ip:util_ds_buf:2.1\
anton-potocnik:user:axi_bram_reader:1.0\
pavel-demin:user:axi_cfg_register:1.0\
xilinx.com:ip:axi_gpio:2.0\
xilinx.com:ip:proc_sys_reset:5.0\
xilinx.com:ip:processing_system7:5.5\
xilinx.com:ip:blk_mem_gen:8.4\
"

   set list_ips_missing ""
   common::send_msg_id "BD_TCL-006" "INFO" "Checking if the following IPs exist in the project's IP catalog: $list_check_ips ."

   foreach ip_vlnv $list_check_ips {
      set ip_obj [get_ipdefs -all $ip_vlnv]
      if { $ip_obj eq "" } {
         lappend list_ips_missing $ip_vlnv
      }
   }

   if { $list_ips_missing ne "" } {
      catch {common::send_msg_id "BD_TCL-115" "ERROR" "The following IPs are not found in the IP Catalog:\n  $list_ips_missing\n\nResolution: Please add the repository containing the IP(s) to the project." }
      set bCheckIPsPassed 0
   }

}

##################################################################
# CHECK Modules
##################################################################
set bCheckModules 1
if { $bCheckModules == 1 } {
   set list_check_mods "\ 
VolumeAdjuster16_14\
axis_dc_filter\
axis_decimator\
axis_sc28_to_14\
cfg_select\
cfg_select\
cfg_select\
cfg_select\
cfg_select\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
led_connect\
lms_phase_amplitude_detector\
controller_pi\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_select\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
logic_or\
controller_pi\
phase_unwrap\
McBSP_controller\
McBSP_io_connect\
ScaleAndAdjust\
axis_4s_combine\
axis_bram_push\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
cfg_to_axis\
"

   set list_mods_missing ""
   common::send_msg_id "BD_TCL-006" "INFO" "Checking if the following modules exist in the project's sources: $list_check_mods ."

   foreach mod_vlnv $list_check_mods {
      if { [can_resolve_reference $mod_vlnv] == 0 } {
         lappend list_mods_missing $mod_vlnv
      }
   }

   if { $list_mods_missing ne "" } {
      catch {common::send_msg_id "BD_TCL-115" "ERROR" "The following module(s) are not found in the project: $list_mods_missing" }
      common::send_msg_id "BD_TCL-008" "INFO" "Please add source files for the missing module(s) above."
      set bCheckIPsPassed 0
   }
}

if { $bCheckIPsPassed != 1 } {
  common::send_msg_id "BD_TCL-1003" "WARNING" "Will not continue with creation of design due to the error(s) above."
  return 3
}

##################################################################
# DESIGN PROCs
##################################################################


# Hierarchical cell: PS_data_transport
proc create_hier_cell_PS_data_transport { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_msg_id "BD_TCL-102" "ERROR" "create_hier_cell_PS_data_transport() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:bram_rtl:1.0 BRAM_PORTB

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXIS_aux

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXIS1

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXIS2

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXIS3

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXIS4

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXIS5

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXIS6

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXIS7

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXIS8


  # Create pins
  create_bd_pin -dir I -type clk a2_clk
  create_bd_pin -dir I -type clk a_clk
  create_bd_pin -dir I -from 1023 -to 0 cfg
  create_bd_pin -dir O -from 31 -to 0 dbgA
  create_bd_pin -dir O -from 31 -to 0 dbgB
  create_bd_pin -dir O -from 31 -to 0 debug
  create_bd_pin -dir IO -from 7 -to 0 exp_n_io
  create_bd_pin -dir IO -from 7 -to 0 exp_p_io
  create_bd_pin -dir O finished_state
  create_bd_pin -dir O init_state
  create_bd_pin -dir O -from 31 -to 0 writeposition

  # Create instance: McBSP_controller_0, and set properties
  set block_name McBSP_controller
  set block_cell_name McBSP_controller_0
  if { [catch {set McBSP_controller_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $McBSP_controller_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: McBSP_io_connect_0, and set properties
  set block_name McBSP_io_connect
  set block_cell_name McBSP_io_connect_0
  if { [catch {set McBSP_io_connect_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $McBSP_io_connect_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: ScaleAndAdjust_0, and set properties
  set block_name ScaleAndAdjust
  set block_cell_name ScaleAndAdjust_0
  if { [catch {set ScaleAndAdjust_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $ScaleAndAdjust_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.GAIN_DATA_Q {15} \
 ] $ScaleAndAdjust_0

  # Create instance: axis_4s_combine_0, and set properties
  set block_name axis_4s_combine
  set block_cell_name axis_4s_combine_0
  if { [catch {set axis_4s_combine_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $axis_4s_combine_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.BRAM_ADDR_WIDTH {15} \
   CONFIG.BRAM_DATA_WIDTH {64} \
 ] $axis_4s_combine_0

  # Create instance: axis_bram_push2ch64, and set properties
  set block_name axis_bram_push
  set block_cell_name axis_bram_push2ch64
  if { [catch {set axis_bram_push2ch64 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $axis_bram_push2ch64 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: blk_mem_gen_0, and set properties
  set blk_mem_gen_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:8.4 blk_mem_gen_0 ]
  set_property -dict [ list \
   CONFIG.Algorithm {Minimum_Area} \
   CONFIG.Byte_Size {9} \
   CONFIG.EN_SAFETY_CKT {false} \
   CONFIG.Enable_32bit_Address {false} \
   CONFIG.Enable_B {Use_ENB_Pin} \
   CONFIG.Memory_Type {Simple_Dual_Port_RAM} \
   CONFIG.Operating_Mode_A {READ_FIRST} \
   CONFIG.Port_B_Clock {100} \
   CONFIG.Port_B_Enable_Rate {100} \
   CONFIG.Read_Width_A {64} \
   CONFIG.Read_Width_B {64} \
   CONFIG.Register_PortA_Output_of_Memory_Primitives {false} \
   CONFIG.Register_PortB_Output_of_Memory_Primitives {true} \
   CONFIG.Use_Byte_Write_Enable {false} \
   CONFIG.Use_RSTA_Pin {false} \
   CONFIG.Use_RSTB_Pin {false} \
   CONFIG.Write_Depth_A {32768} \
   CONFIG.Write_Width_A {64} \
   CONFIG.Write_Width_B {64} \
   CONFIG.use_bram_block {Stand_Alone} \
 ] $blk_mem_gen_0

  # Create instance: cfg17_to_transport_auxscale, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg17_to_transport_auxscale
  if { [catch {set cfg17_to_transport_auxscale [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg17_to_transport_auxscale eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {32} \
   CONFIG.SRC_ADDR {17} \
   CONFIG.SRC_BITS {32} \
 ] $cfg17_to_transport_auxscale

  # Create instance: cfg18_19_to_transport_auxcenter, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg18_19_to_transport_auxcenter
  if { [catch {set cfg18_19_to_transport_auxcenter [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg18_19_to_transport_auxcenter eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {48} \
   CONFIG.MAXIS_TDATA_WIDTH {48} \
   CONFIG.SRC_ADDR {18} \
   CONFIG.SRC_BITS {64} \
 ] $cfg18_19_to_transport_auxcenter

  # Create instance: cfg_to_transport_channel_select, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_transport_channel_select
  if { [catch {set cfg_to_transport_channel_select [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_transport_channel_select eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {32} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {9} \
 ] $cfg_to_transport_channel_select

  # Create instance: cfg_to_transport_control, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_transport_control
  if { [catch {set cfg_to_transport_control [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_transport_control eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {32} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {6} \
 ] $cfg_to_transport_control

  # Create instance: cfg_to_transport_decimation, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_transport_decimation
  if { [catch {set cfg_to_transport_decimation [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_transport_decimation eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {32} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {8} \
 ] $cfg_to_transport_decimation

  # Create instance: cfg_to_transport_nsamples, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_transport_nsamples
  if { [catch {set cfg_to_transport_nsamples [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_transport_nsamples eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {32} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {7} \
 ] $cfg_to_transport_nsamples

  # Create interface connections
  connect_bd_intf_net -intf_net Conn1 [get_bd_intf_pins S_AXIS7] [get_bd_intf_pins axis_4s_combine_0/S_AXIS7]
  connect_bd_intf_net -intf_net Conn2 [get_bd_intf_pins S_AXIS8] [get_bd_intf_pins axis_4s_combine_0/S_AXIS8]
  connect_bd_intf_net -intf_net Conn3 [get_bd_intf_pins S_AXIS6] [get_bd_intf_pins axis_4s_combine_0/S_AXIS6]
  connect_bd_intf_net -intf_net PS_Amplitude_Controller_M_AXIS_CONTROL2 [get_bd_intf_pins S_AXIS2] [get_bd_intf_pins axis_4s_combine_0/S_AXIS2]
  connect_bd_intf_net -intf_net PS_Amplitude_Controller_M_AXIS_PASS2 [get_bd_intf_pins S_AXIS5] [get_bd_intf_pins axis_4s_combine_0/S_AXIS5]
  connect_bd_intf_net -intf_net PS_BRAM_PORTA [get_bd_intf_pins BRAM_PORTB] [get_bd_intf_pins blk_mem_gen_0/BRAM_PORTB]
  connect_bd_intf_net -intf_net PS_Phase_Controller_M_AXIS_CONTROL2 [get_bd_intf_pins S_AXIS3] [get_bd_intf_pins axis_4s_combine_0/S_AXIS3]
  connect_bd_intf_net -intf_net PS_Phase_Controller_M_AXIS_PASS2 [get_bd_intf_pins S_AXIS4] [get_bd_intf_pins axis_4s_combine_0/S_AXIS4]
  connect_bd_intf_net -intf_net ScaleAndAdjust_0_M_AXIS [get_bd_intf_pins M_AXIS_aux] [get_bd_intf_pins ScaleAndAdjust_0/M_AXIS]
  connect_bd_intf_net -intf_net axis_4s_combine_0_M_AXIS_AMPL [get_bd_intf_pins McBSP_controller_0/S_AXIS5] [get_bd_intf_pins axis_4s_combine_0/M_AXIS_AMPL]
  connect_bd_intf_net -intf_net axis_4s_combine_0_M_AXIS_CH1 [get_bd_intf_pins McBSP_controller_0/S_AXIS1] [get_bd_intf_pins axis_4s_combine_0/M_AXIS_CH1]
  connect_bd_intf_net -intf_net axis_4s_combine_0_M_AXIS_CH2 [get_bd_intf_pins McBSP_controller_0/S_AXIS2] [get_bd_intf_pins axis_4s_combine_0/M_AXIS_CH2]
  connect_bd_intf_net -intf_net axis_4s_combine_0_M_AXIS_CH3 [get_bd_intf_pins McBSP_controller_0/S_AXIS3] [get_bd_intf_pins axis_4s_combine_0/M_AXIS_CH3]
  connect_bd_intf_net -intf_net axis_4s_combine_0_M_AXIS_CH4 [get_bd_intf_pins McBSP_controller_0/S_AXIS4] [get_bd_intf_pins axis_4s_combine_0/M_AXIS_CH4]
  connect_bd_intf_net -intf_net axis_4s_combine_0_M_AXIS_FREQ31_0 [get_bd_intf_pins McBSP_controller_0/S_AXIS8] [get_bd_intf_pins axis_4s_combine_0/M_AXIS_FREQ31_0]
  connect_bd_intf_net -intf_net axis_4s_combine_0_M_AXIS_FREQ63_32 [get_bd_intf_pins McBSP_controller_0/S_AXIS7] [get_bd_intf_pins axis_4s_combine_0/M_AXIS_FREQ63_32]
  connect_bd_intf_net -intf_net axis_4s_combine_0_M_AXIS_PHASE [get_bd_intf_pins McBSP_controller_0/S_AXIS6] [get_bd_intf_pins axis_4s_combine_0/M_AXIS_PHASE]
  connect_bd_intf_net -intf_net axis_4s_combine_0_M_AXIS_aux [get_bd_intf_pins ScaleAndAdjust_0/S_AXIS] [get_bd_intf_pins axis_4s_combine_0/M_AXIS_aux]
  connect_bd_intf_net -intf_net lms_phase_amplitude_detector_0_M_AXIS_SIGNAL [get_bd_intf_pins S_AXIS1] [get_bd_intf_pins axis_4s_combine_0/S_AXIS1]

  # Create port connections
  connect_bd_net -net McBSP_controller_0_dbgA [get_bd_pins dbgA] [get_bd_pins McBSP_controller_0/dbgA]
  connect_bd_net -net McBSP_controller_0_dbgB [get_bd_pins dbgB] [get_bd_pins McBSP_controller_0/dbgB]
  connect_bd_net -net McBSP_controller_0_mcbsp_data_clkr [get_bd_pins McBSP_controller_0/mcbsp_data_clkr] [get_bd_pins McBSP_io_connect_0/McBSP_clkr]
  connect_bd_net -net McBSP_controller_0_mcbsp_data_frm [get_bd_pins McBSP_controller_0/mcbsp_data_frm] [get_bd_pins McBSP_io_connect_0/McBSP_frm]
  connect_bd_net -net McBSP_controller_0_mcbsp_data_fsx [get_bd_pins McBSP_controller_0/mcbsp_data_fsx] [get_bd_pins McBSP_io_connect_0/McBSP_fsx]
  connect_bd_net -net McBSP_controller_0_mcbsp_data_tx [get_bd_pins McBSP_controller_0/mcbsp_data_tx] [get_bd_pins McBSP_io_connect_0/McBSP_tx]
  connect_bd_net -net McBSP_controller_0_trigger [get_bd_pins McBSP_controller_0/trigger] [get_bd_pins axis_4s_combine_0/ext_trigger]
  connect_bd_net -net McBSP_io_connect_0_McBSP_clk [get_bd_pins McBSP_controller_0/mcbsp_clk] [get_bd_pins McBSP_io_connect_0/McBSP_clk]
  connect_bd_net -net McBSP_io_connect_0_McBSP_fs [get_bd_pins McBSP_controller_0/mcbsp_frame_start] [get_bd_pins McBSP_io_connect_0/McBSP_fs]
  connect_bd_net -net McBSP_io_connect_0_McBSP_nrx [get_bd_pins McBSP_controller_0/mcbsp_data_nrx] [get_bd_pins McBSP_io_connect_0/McBSP_nrx]
  connect_bd_net -net McBSP_io_connect_0_McBSP_rx [get_bd_pins McBSP_controller_0/mcbsp_data_rx] [get_bd_pins McBSP_io_connect_0/McBSP_rx]
  connect_bd_net -net McBSP_io_connect_0_RP_exp_in [get_bd_pins McBSP_io_connect_0/RP_exp_in] [get_bd_pins axis_4s_combine_0/rp_digital_in]
  connect_bd_net -net Net [get_bd_pins exp_p_io] [get_bd_pins McBSP_io_connect_0/exp_p_io]
  connect_bd_net -net Net1 [get_bd_pins exp_n_io] [get_bd_pins McBSP_io_connect_0/exp_n_io]
  connect_bd_net -net a2_clk_1 [get_bd_pins a2_clk] [get_bd_pins axis_bram_push2ch64/a2_clk]
  connect_bd_net -net axis_4s_combine_0_BR_ch1s [get_bd_pins axis_4s_combine_0/BR_ch1s] [get_bd_pins axis_bram_push2ch64/ch1s]
  connect_bd_net -net axis_4s_combine_0_BR_ch2s [get_bd_pins axis_4s_combine_0/BR_ch2s] [get_bd_pins axis_bram_push2ch64/ch2s]
  connect_bd_net -net axis_4s_combine_0_BR_next [get_bd_pins axis_4s_combine_0/BR_next] [get_bd_pins axis_bram_push2ch64/push_next]
  connect_bd_net -net axis_4s_combine_0_BR_reset [get_bd_pins axis_4s_combine_0/BR_reset] [get_bd_pins axis_bram_push2ch64/reset]
  connect_bd_net -net axis_4s_combine_0_bram_porta_addr [get_bd_pins axis_bram_push2ch64/BRAM_PORTA_addr] [get_bd_pins blk_mem_gen_0/addra]
  connect_bd_net -net axis_4s_combine_0_bram_porta_clk [get_bd_pins axis_bram_push2ch64/BRAM_PORTA_clk] [get_bd_pins blk_mem_gen_0/clka]
  connect_bd_net -net axis_4s_combine_0_bram_porta_en [get_bd_pins axis_bram_push2ch64/BRAM_PORTA_en] [get_bd_pins blk_mem_gen_0/ena]
  connect_bd_net -net axis_4s_combine_0_bram_porta_we [get_bd_pins axis_bram_push2ch64/BRAM_PORTA_we] [get_bd_pins blk_mem_gen_0/wea]
  connect_bd_net -net axis_4s_combine_0_bram_porta_wrdata [get_bd_pins axis_bram_push2ch64/BRAM_PORTA_din] [get_bd_pins blk_mem_gen_0/dina]
  connect_bd_net -net axis_4s_combine_0_debug [get_bd_pins debug] [get_bd_pins axis_4s_combine_0/debug]
  connect_bd_net -net axis_4s_combine_0_finished_state [get_bd_pins finished_state] [get_bd_pins axis_4s_combine_0/finished_state]
  connect_bd_net -net axis_4s_combine_0_init_state [get_bd_pins init_state] [get_bd_pins axis_4s_combine_0/init_state]
  connect_bd_net -net axis_4s_combine_0_writeposition [get_bd_pins writeposition] [get_bd_pins axis_4s_combine_0/writeposition]
  connect_bd_net -net axis_bram_push_0_ready [get_bd_pins axis_4s_combine_0/BR_ready] [get_bd_pins axis_bram_push2ch64/ready]
  connect_bd_net -net axis_red_pitaya_adc_0_adc_clk [get_bd_pins a_clk] [get_bd_pins McBSP_controller_0/a_clk] [get_bd_pins ScaleAndAdjust_0/a_clk] [get_bd_pins axis_4s_combine_0/a_clk] [get_bd_pins cfg17_to_transport_auxscale/a_clk] [get_bd_pins cfg18_19_to_transport_auxcenter/a_clk] [get_bd_pins cfg_to_transport_channel_select/a_clk] [get_bd_pins cfg_to_transport_control/a_clk] [get_bd_pins cfg_to_transport_decimation/a_clk] [get_bd_pins cfg_to_transport_nsamples/a_clk]
  connect_bd_net -net cfg18_19_to_transport_auxcenter_data [get_bd_pins axis_4s_combine_0/axis3_center] [get_bd_pins cfg18_19_to_transport_auxcenter/data]
  connect_bd_net -net cfg_1 [get_bd_pins cfg] [get_bd_pins cfg17_to_transport_auxscale/cfg] [get_bd_pins cfg18_19_to_transport_auxcenter/cfg] [get_bd_pins cfg_to_transport_channel_select/cfg] [get_bd_pins cfg_to_transport_control/cfg] [get_bd_pins cfg_to_transport_decimation/cfg] [get_bd_pins cfg_to_transport_nsamples/cfg]
  connect_bd_net -net cfg_to_axis_0_data [get_bd_pins ScaleAndAdjust_0/gain] [get_bd_pins cfg17_to_transport_auxscale/data]
  connect_bd_net -net cfg_to_channel_select_data [get_bd_pins axis_4s_combine_0/channel_selector] [get_bd_pins cfg_to_transport_channel_select/data]
  connect_bd_net -net cfg_to_transport_channel_select1_data [get_bd_pins axis_4s_combine_0/nsamples] [get_bd_pins cfg_to_transport_nsamples/data]
  connect_bd_net -net cfg_to_transport_control_data [get_bd_pins axis_4s_combine_0/operation] [get_bd_pins cfg_to_transport_control/data]
  connect_bd_net -net cfg_to_transport_data [get_bd_pins axis_4s_combine_0/ndecimate] [get_bd_pins cfg_to_transport_decimation/data]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: PS_Phase_Controller
proc create_hier_cell_PS_Phase_Controller { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_msg_id "BD_TCL-102" "ERROR" "create_hier_cell_PS_Phase_Controller() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXIS_CONTROL

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXIS_CONTROL2

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXIS_PASS

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXIS_PASS2

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXIS


  # Create pins
  create_bd_pin -dir I -type clk aclk
  create_bd_pin -dir I -from 1023 -to 0 cfg
  create_bd_pin -dir O -from 47 -to 0 data_lower
  create_bd_pin -dir I enable
  create_bd_pin -dir O -from 31 -to 0 mon_control
  create_bd_pin -dir O -from 31 -to 0 mon_control_B
  create_bd_pin -dir O -from 31 -to 0 mon_control_lower32
  create_bd_pin -dir O -from 31 -to 0 mon_signal
  create_bd_pin -dir O status_max
  create_bd_pin -dir O status_min

  # Create instance: cfg_select_phase_unwrapping, and set properties
  set block_name cfg_select
  set block_cell_name cfg_select_phase_unwrapping
  if { [catch {set cfg_select_phase_unwrapping [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_select_phase_unwrapping eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.CFG_SWBIT {2} \
   CONFIG.SRC_ADDR {3} \
 ] $cfg_select_phase_unwrapping

  # Create instance: cfg_to_axis_PhaseInc, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_axis_PhaseInc
  if { [catch {set cfg_to_axis_PhaseInc [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_axis_PhaseInc eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {48} \
   CONFIG.MAXIS_TDATA_WIDTH {48} \
   CONFIG.SRC_BITS {64} \
 ] $cfg_to_axis_PhaseInc

  # Create instance: cfg_to_ph_ci, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_ph_ci
  if { [catch {set cfg_to_ph_ci [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_ph_ci eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {32} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {12} \
   CONFIG.SRC_BITS {32} \
 ] $cfg_to_ph_ci

  # Create instance: cfg_to_ph_cp, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_ph_cp
  if { [catch {set cfg_to_ph_cp [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_ph_cp eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {32} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {11} \
   CONFIG.SRC_BITS {32} \
 ] $cfg_to_ph_cp

  # Create instance: cfg_to_ph_lower, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_ph_lower
  if { [catch {set cfg_to_ph_lower [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_ph_lower eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {48} \
   CONFIG.MAXIS_TDATA_WIDTH {48} \
   CONFIG.SRC_ADDR {15} \
   CONFIG.SRC_BITS {64} \
 ] $cfg_to_ph_lower

  # Create instance: cfg_to_ph_setpoint, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_ph_setpoint
  if { [catch {set cfg_to_ph_setpoint [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_ph_setpoint eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {32} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {10} \
 ] $cfg_to_ph_setpoint

  # Create instance: cfg_to_ph_upper, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_ph_upper
  if { [catch {set cfg_to_ph_upper [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_ph_upper eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {48} \
   CONFIG.MAXIS_TDATA_WIDTH {48} \
   CONFIG.SRC_ADDR {13} \
   CONFIG.SRC_BITS {64} \
 ] $cfg_to_ph_upper

  # Create instance: logic_or, and set properties
  set block_name logic_or
  set block_cell_name logic_or
  if { [catch {set logic_or [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $logic_or eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: phase_controller, and set properties
  set block_name controller_pi
  set block_cell_name phase_controller
  if { [catch {set phase_controller [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $phase_controller eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.AMCONTROL_ALLOW_NEG_SPECIAL {0} \
   CONFIG.AUTO_RESET_AT_LIMIT {1} \
   CONFIG.AXIS_TDATA_WIDTH {32} \
   CONFIG.CEXTEND {1} \
   CONFIG.CONTROL2_OUT_WIDTH {44} \
   CONFIG.CONTROL2_WIDTH {75} \
   CONFIG.CONTROL2_WIDTH_X {71} \
   CONFIG.CONTROL_WIDTH {44} \
   CONFIG.M_AXIS_CONTROL2_TDATA_WIDTH {48} \
   CONFIG.M_AXIS_CONTROL_TDATA_WIDTH {48} \
 ] $phase_controller

  # Create instance: phase_unwrap, and set properties
  set block_name phase_unwrap
  set block_cell_name phase_unwrap
  if { [catch {set phase_unwrap [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $phase_unwrap eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create interface connections
  connect_bd_intf_net -intf_net Conn1 [get_bd_intf_pins M_AXIS_CONTROL2] [get_bd_intf_pins phase_controller/M_AXIS_CONTROL2]
  connect_bd_intf_net -intf_net Conn2 [get_bd_intf_pins M_AXIS_PASS2] [get_bd_intf_pins phase_controller/M_AXIS_PASS2]
  connect_bd_intf_net -intf_net S_AXIS_1 [get_bd_intf_pins S_AXIS] [get_bd_intf_pins phase_unwrap/S_AXIS]
  connect_bd_intf_net -intf_net cfg_to_axis_PhaseInc_M_AXIS [get_bd_intf_pins cfg_to_axis_PhaseInc/M_AXIS] [get_bd_intf_pins phase_controller/S_AXIS_reset]
  connect_bd_intf_net -intf_net phase_controller_M_AXIS_CONTROL [get_bd_intf_pins M_AXIS_CONTROL] [get_bd_intf_pins phase_controller/M_AXIS_CONTROL]
  connect_bd_intf_net -intf_net phase_controller_M_AXIS_PASS [get_bd_intf_pins M_AXIS_PASS] [get_bd_intf_pins phase_controller/M_AXIS_PASS]
  connect_bd_intf_net -intf_net phase_unwrap_0_M_AXIS [get_bd_intf_pins phase_controller/S_AXIS] [get_bd_intf_pins phase_unwrap/M_AXIS]

  # Create port connections
  connect_bd_net -net Net [get_bd_pins cfg] [get_bd_pins cfg_select_phase_unwrapping/cfg] [get_bd_pins cfg_to_axis_PhaseInc/cfg] [get_bd_pins cfg_to_ph_ci/cfg] [get_bd_pins cfg_to_ph_cp/cfg] [get_bd_pins cfg_to_ph_lower/cfg] [get_bd_pins cfg_to_ph_setpoint/cfg] [get_bd_pins cfg_to_ph_upper/cfg]
  connect_bd_net -net axis_red_pitaya_adc_0_adc_clk [get_bd_pins aclk] [get_bd_pins cfg_to_axis_PhaseInc/a_clk] [get_bd_pins cfg_to_ph_ci/a_clk] [get_bd_pins cfg_to_ph_cp/a_clk] [get_bd_pins cfg_to_ph_lower/a_clk] [get_bd_pins cfg_to_ph_setpoint/a_clk] [get_bd_pins cfg_to_ph_upper/a_clk] [get_bd_pins phase_controller/aclk] [get_bd_pins phase_unwrap/aclk]
  connect_bd_net -net cfg_select_phase_unwrapping_status [get_bd_pins cfg_select_phase_unwrapping/status] [get_bd_pins logic_or/B]
  connect_bd_net -net cfg_to_am_setpoint_data [get_bd_pins cfg_to_ph_setpoint/data] [get_bd_pins phase_controller/setpoint]
  connect_bd_net -net cfg_to_ph_ci_data [get_bd_pins cfg_to_ph_ci/data] [get_bd_pins phase_controller/ci]
  connect_bd_net -net cfg_to_ph_cp_data [get_bd_pins cfg_to_ph_cp/data] [get_bd_pins phase_controller/cp]
  connect_bd_net -net cfg_to_ph_lower_data [get_bd_pins cfg_to_ph_upper/data] [get_bd_pins phase_controller/limit_upper]
  connect_bd_net -net cfg_to_ph_upper_data [get_bd_pins data_lower] [get_bd_pins cfg_to_ph_lower/data] [get_bd_pins phase_controller/limit_lower]
  connect_bd_net -net enable_1 [get_bd_pins enable] [get_bd_pins logic_or/A] [get_bd_pins phase_controller/enable]
  connect_bd_net -net logic_or_0_AorB [get_bd_pins logic_or/AorB] [get_bd_pins phase_unwrap/enable]
  connect_bd_net -net phase_controller_mon_control [get_bd_pins mon_control] [get_bd_pins phase_controller/mon_control]
  connect_bd_net -net phase_controller_mon_control_B [get_bd_pins mon_control_B] [get_bd_pins phase_controller/mon_control_B]
  connect_bd_net -net phase_controller_mon_control_lower32 [get_bd_pins mon_control_lower32] [get_bd_pins phase_controller/mon_control_lower32]
  connect_bd_net -net phase_controller_mon_signal [get_bd_pins mon_signal] [get_bd_pins phase_controller/mon_signal]
  connect_bd_net -net phase_controller_status_max [get_bd_pins status_max] [get_bd_pins phase_controller/status_max]
  connect_bd_net -net phase_controller_status_min [get_bd_pins status_min] [get_bd_pins phase_controller/status_min]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: PS_Amplitude_Controller
proc create_hier_cell_PS_Amplitude_Controller { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_msg_id "BD_TCL-102" "ERROR" "create_hier_cell_PS_Amplitude_Controller() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXIS_CONTROL

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXIS_CONTROL2

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXIS_PASS

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXIS_PASS2

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXIS


  # Create pins
  create_bd_pin -dir I -type clk aclk
  create_bd_pin -dir I -from 1023 -to 0 cfg
  create_bd_pin -dir I enable
  create_bd_pin -dir O -from 31 -to 0 mon_control
  create_bd_pin -dir O -from 31 -to 0 mon_control_lower32
  create_bd_pin -dir O -from 31 -to 0 mon_signal
  create_bd_pin -dir O status_max
  create_bd_pin -dir O status_min

  # Create instance: amplitude_controller, and set properties
  set block_name controller_pi
  set block_cell_name amplitude_controller
  if { [catch {set amplitude_controller [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $amplitude_controller eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.AMCONTROL_ALLOW_NEG_SPECIAL {1} \
   CONFIG.AXIS_TDATA_WIDTH {24} \
   CONFIG.CONTROL2_OUT_WIDTH {32} \
   CONFIG.CONTROL2_WIDTH {56} \
   CONFIG.CONTROL2_WIDTH_X {52} \
   CONFIG.CONTROL_WIDTH {16} \
   CONFIG.M_AXIS_CONTROL_TDATA_WIDTH {16} \
 ] $amplitude_controller

  # Create instance: cfg_to_am_ci, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_am_ci
  if { [catch {set cfg_to_am_ci [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_am_ci eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {32} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {22} \
 ] $cfg_to_am_ci

  # Create instance: cfg_to_am_cp, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_am_cp
  if { [catch {set cfg_to_am_cp [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_am_cp eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {32} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {21} \
 ] $cfg_to_am_cp

  # Create instance: cfg_to_am_lower, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_am_lower
  if { [catch {set cfg_to_am_lower [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_am_lower eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {16} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {25} \
 ] $cfg_to_am_lower

  # Create instance: cfg_to_am_setpoint, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_am_setpoint
  if { [catch {set cfg_to_am_setpoint [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_am_setpoint eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {24} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {20} \
 ] $cfg_to_am_setpoint

  # Create instance: cfg_to_am_upper, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_am_upper
  if { [catch {set cfg_to_am_upper [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_am_upper eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {16} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {23} \
 ] $cfg_to_am_upper

  # Create instance: cfg_to_axis_volume, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_axis_volume
  if { [catch {set cfg_to_axis_volume [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_axis_volume eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {16} \
   CONFIG.MAXIS_TDATA_WIDTH {16} \
   CONFIG.SRC_ADDR {2} \
 ] $cfg_to_axis_volume

  # Create interface connections
  connect_bd_intf_net -intf_net Conn1 [get_bd_intf_pins M_AXIS_CONTROL2] [get_bd_intf_pins amplitude_controller/M_AXIS_CONTROL2]
  connect_bd_intf_net -intf_net Conn2 [get_bd_intf_pins M_AXIS_PASS2] [get_bd_intf_pins amplitude_controller/M_AXIS_PASS2]
  connect_bd_intf_net -intf_net amplitude_controller_M_AXIS_CONTROL [get_bd_intf_pins M_AXIS_CONTROL] [get_bd_intf_pins amplitude_controller/M_AXIS_CONTROL]
  connect_bd_intf_net -intf_net amplitude_controller_M_AXIS_PASS [get_bd_intf_pins M_AXIS_PASS] [get_bd_intf_pins amplitude_controller/M_AXIS_PASS]
  connect_bd_intf_net -intf_net cfg_to_axis_volume_M_AXIS [get_bd_intf_pins amplitude_controller/S_AXIS_reset] [get_bd_intf_pins cfg_to_axis_volume/M_AXIS]
  connect_bd_intf_net -intf_net cordic_sqrt_M_AXIS_DOUT [get_bd_intf_pins S_AXIS] [get_bd_intf_pins amplitude_controller/S_AXIS]

  # Create port connections
  connect_bd_net -net PS_cfg_data [get_bd_pins cfg] [get_bd_pins cfg_to_am_ci/cfg] [get_bd_pins cfg_to_am_cp/cfg] [get_bd_pins cfg_to_am_lower/cfg] [get_bd_pins cfg_to_am_setpoint/cfg] [get_bd_pins cfg_to_am_upper/cfg] [get_bd_pins cfg_to_axis_volume/cfg]
  connect_bd_net -net amplitude_controller_mon_control [get_bd_pins mon_control] [get_bd_pins amplitude_controller/mon_control]
  connect_bd_net -net amplitude_controller_mon_control_lower32 [get_bd_pins mon_control_lower32] [get_bd_pins amplitude_controller/mon_control_lower32]
  connect_bd_net -net amplitude_controller_mon_signal [get_bd_pins mon_signal] [get_bd_pins amplitude_controller/mon_signal]
  connect_bd_net -net amplitude_controller_status_max [get_bd_pins status_max] [get_bd_pins amplitude_controller/status_max]
  connect_bd_net -net amplitude_controller_status_min [get_bd_pins status_min] [get_bd_pins amplitude_controller/status_min]
  connect_bd_net -net axis_red_pitaya_adc_0_adc_clk [get_bd_pins aclk] [get_bd_pins amplitude_controller/aclk] [get_bd_pins cfg_to_am_ci/a_clk] [get_bd_pins cfg_to_am_cp/a_clk] [get_bd_pins cfg_to_am_lower/a_clk] [get_bd_pins cfg_to_am_setpoint/a_clk] [get_bd_pins cfg_to_am_upper/a_clk] [get_bd_pins cfg_to_axis_volume/a_clk]
  connect_bd_net -net cfg_to_am_ci_data [get_bd_pins amplitude_controller/ci] [get_bd_pins cfg_to_am_ci/data]
  connect_bd_net -net cfg_to_am_cp_data [get_bd_pins amplitude_controller/cp] [get_bd_pins cfg_to_am_cp/data]
  connect_bd_net -net cfg_to_am_lower_data [get_bd_pins amplitude_controller/limit_lower] [get_bd_pins cfg_to_am_lower/data]
  connect_bd_net -net cfg_to_am_setpoint_data [get_bd_pins amplitude_controller/setpoint] [get_bd_pins cfg_to_am_setpoint/data]
  connect_bd_net -net cfg_to_am_upper_data [get_bd_pins amplitude_controller/limit_upper] [get_bd_pins cfg_to_am_upper/data]
  connect_bd_net -net enable_1 [get_bd_pins enable] [get_bd_pins amplitude_controller/enable]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: PS
proc create_hier_cell_PS { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_msg_id "BD_TCL-102" "ERROR" "create_hier_cell_PS() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:bram_rtl:1.0 BRAM_PORTA

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:ddrx_rtl:1.0 DDR

  create_bd_intf_pin -mode Master -vlnv xilinx.com:display_processing_system7:fixedio_rtl:1.0 FIXED_IO

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M08_AXI

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M09_AXI

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M10_AXI


  # Create pins
  create_bd_pin -dir O -type clk FCLK_CLK0
  create_bd_pin -dir I SPI0_MISO_I
  create_bd_pin -dir O SPI0_MOSI_O
  create_bd_pin -dir I SPI0_SCLK_I
  create_bd_pin -dir O SPI0_SCLK_O
  create_bd_pin -dir O SPI0_SS_O
  create_bd_pin -dir O -from 1023 -to 0 cfg_data
  create_bd_pin -dir I -from 31 -to 0 gpio_io_0_x1
  create_bd_pin -dir I -from 31 -to 0 gpio_io_0_x2
  create_bd_pin -dir I -from 31 -to 0 gpio_io_1_x3
  create_bd_pin -dir I -from 31 -to 0 gpio_io_1_x4
  create_bd_pin -dir I -from 31 -to 0 gpio_io_2_x5
  create_bd_pin -dir I -from 31 -to 0 gpio_io_2_x6
  create_bd_pin -dir I -from 31 -to 0 gpio_io_3_x7
  create_bd_pin -dir I -from 31 -to 0 gpio_io_3_x8
  create_bd_pin -dir I -from 31 -to 0 gpio_io_4_x9
  create_bd_pin -dir I -from 31 -to 0 gpio_io_4_x10
  create_bd_pin -dir I -from 31 -to 0 gpio_io_5_x11
  create_bd_pin -dir I -from 31 -to 0 gpio_io_5_x12
  create_bd_pin -dir O -from 0 -to 0 -type rst interconnect_aresetn1
  create_bd_pin -dir O -from 0 -to 0 -type rst peripheral_aresetn
  create_bd_pin -dir O -from 0 -to 0 -type rst peripheral_aresetn1
  create_bd_pin -dir I -type clk slowest_sync_clk

  # Create instance: axi_bram_reader_0, and set properties
  set axi_bram_reader_0 [ create_bd_cell -type ip -vlnv anton-potocnik:user:axi_bram_reader:1.0 axi_bram_reader_0 ]
  set_property -dict [ list \
   CONFIG.BRAM_ADDR_WIDTH {15} \
   CONFIG.BRAM_DATA_WIDTH {64} \
   CONFIG.C_S00_AXI_ADDR_WIDTH {17} \
 ] $axi_bram_reader_0

  # Create instance: axi_cfg_register_0, and set properties
  set axi_cfg_register_0 [ create_bd_cell -type ip -vlnv pavel-demin:user:axi_cfg_register:1.0 axi_cfg_register_0 ]

  # Create instance: axi_gpio_0, and set properties
  set axi_gpio_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_0 ]
  set_property -dict [ list \
   CONFIG.C_ALL_INPUTS {1} \
   CONFIG.C_ALL_INPUTS_2 {1} \
   CONFIG.C_IS_DUAL {1} \
 ] $axi_gpio_0

  # Create instance: axi_gpio_1, and set properties
  set axi_gpio_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_1 ]
  set_property -dict [ list \
   CONFIG.C_ALL_INPUTS {1} \
   CONFIG.C_ALL_INPUTS_2 {1} \
   CONFIG.C_IS_DUAL {1} \
 ] $axi_gpio_1

  # Create instance: axi_gpio_2, and set properties
  set axi_gpio_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_2 ]
  set_property -dict [ list \
   CONFIG.C_ALL_INPUTS {1} \
   CONFIG.C_ALL_INPUTS_2 {1} \
   CONFIG.C_IS_DUAL {1} \
 ] $axi_gpio_2

  # Create instance: axi_gpio_3, and set properties
  set axi_gpio_3 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_3 ]
  set_property -dict [ list \
   CONFIG.C_ALL_INPUTS {1} \
   CONFIG.C_ALL_INPUTS_2 {1} \
   CONFIG.C_IS_DUAL {1} \
 ] $axi_gpio_3

  # Create instance: axi_gpio_4, and set properties
  set axi_gpio_4 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_4 ]
  set_property -dict [ list \
   CONFIG.C_ALL_INPUTS {1} \
   CONFIG.C_ALL_INPUTS_2 {1} \
   CONFIG.C_IS_DUAL {1} \
 ] $axi_gpio_4

  # Create instance: axi_gpio_5, and set properties
  set axi_gpio_5 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_5 ]
  set_property -dict [ list \
   CONFIG.C_ALL_INPUTS {1} \
   CONFIG.C_ALL_INPUTS_2 {1} \
   CONFIG.C_IS_DUAL {1} \
 ] $axi_gpio_5

  # Create instance: axi_interconnect_0, and set properties
  set axi_interconnect_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_0 ]
  set_property -dict [ list \
   CONFIG.NUM_MI {11} \
 ] $axi_interconnect_0

  # Create instance: proc_sys_reset_0, and set properties
  set proc_sys_reset_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 proc_sys_reset_0 ]

  # Create instance: processing_system7_0, and set properties
  set processing_system7_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0 ]
  set_property -dict [ list \
   CONFIG.PCW_ACT_APU_PERIPHERAL_FREQMHZ {666.666687} \
   CONFIG.PCW_ACT_CAN_PERIPHERAL_FREQMHZ {10.000000} \
   CONFIG.PCW_ACT_DCI_PERIPHERAL_FREQMHZ {10.158730} \
   CONFIG.PCW_ACT_ENET0_PERIPHERAL_FREQMHZ {125.000000} \
   CONFIG.PCW_ACT_ENET1_PERIPHERAL_FREQMHZ {10.000000} \
   CONFIG.PCW_ACT_FPGA0_PERIPHERAL_FREQMHZ {100.000000} \
   CONFIG.PCW_ACT_FPGA1_PERIPHERAL_FREQMHZ {20.000000} \
   CONFIG.PCW_ACT_FPGA2_PERIPHERAL_FREQMHZ {10.000000} \
   CONFIG.PCW_ACT_FPGA3_PERIPHERAL_FREQMHZ {10.000000} \
   CONFIG.PCW_ACT_PCAP_PERIPHERAL_FREQMHZ {200.000000} \
   CONFIG.PCW_ACT_QSPI_PERIPHERAL_FREQMHZ {125.000000} \
   CONFIG.PCW_ACT_SDIO_PERIPHERAL_FREQMHZ {100.000000} \
   CONFIG.PCW_ACT_SMC_PERIPHERAL_FREQMHZ {10.000000} \
   CONFIG.PCW_ACT_SPI_PERIPHERAL_FREQMHZ {166.666672} \
   CONFIG.PCW_ACT_TPIU_PERIPHERAL_FREQMHZ {200.000000} \
   CONFIG.PCW_ACT_TTC0_CLK0_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_ACT_TTC0_CLK1_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_ACT_TTC0_CLK2_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_ACT_TTC1_CLK0_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_ACT_TTC1_CLK1_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_ACT_TTC1_CLK2_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_ACT_TTC_PERIPHERAL_FREQMHZ {50} \
   CONFIG.PCW_ACT_UART_PERIPHERAL_FREQMHZ {100.000000} \
   CONFIG.PCW_ACT_USB0_PERIPHERAL_FREQMHZ {60} \
   CONFIG.PCW_ACT_USB1_PERIPHERAL_FREQMHZ {60} \
   CONFIG.PCW_ACT_WDT_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_APU_CLK_RATIO_ENABLE {6:2:1} \
   CONFIG.PCW_APU_PERIPHERAL_FREQMHZ {666.666666} \
   CONFIG.PCW_ARMPLL_CTRL_FBDIV {40} \
   CONFIG.PCW_CAN0_GRP_CLK_ENABLE {0} \
   CONFIG.PCW_CAN0_PERIPHERAL_CLKSRC {External} \
   CONFIG.PCW_CAN0_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_CAN1_GRP_CLK_ENABLE {0} \
   CONFIG.PCW_CAN1_PERIPHERAL_CLKSRC {External} \
   CONFIG.PCW_CAN1_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_CAN_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_CAN_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_CAN_PERIPHERAL_DIVISOR1 {1} \
   CONFIG.PCW_CAN_PERIPHERAL_FREQMHZ {100} \
   CONFIG.PCW_CAN_PERIPHERAL_VALID {0} \
   CONFIG.PCW_CLK0_FREQ {100000000} \
   CONFIG.PCW_CLK1_FREQ {20000000} \
   CONFIG.PCW_CLK2_FREQ {10000000} \
   CONFIG.PCW_CLK3_FREQ {10000000} \
   CONFIG.PCW_CPU_CPU_6X4X_MAX_RANGE {667} \
   CONFIG.PCW_CPU_CPU_PLL_FREQMHZ {1333.333} \
   CONFIG.PCW_CPU_PERIPHERAL_CLKSRC {ARM PLL} \
   CONFIG.PCW_CPU_PERIPHERAL_DIVISOR0 {2} \
   CONFIG.PCW_CRYSTAL_PERIPHERAL_FREQMHZ {33.333333} \
   CONFIG.PCW_DCI_PERIPHERAL_CLKSRC {DDR PLL} \
   CONFIG.PCW_DCI_PERIPHERAL_DIVISOR0 {15} \
   CONFIG.PCW_DCI_PERIPHERAL_DIVISOR1 {7} \
   CONFIG.PCW_DCI_PERIPHERAL_FREQMHZ {10.159} \
   CONFIG.PCW_DDRPLL_CTRL_FBDIV {32} \
   CONFIG.PCW_DDR_DDR_PLL_FREQMHZ {1066.667} \
   CONFIG.PCW_DDR_HPRLPR_QUEUE_PARTITION {HPR(0)/LPR(32)} \
   CONFIG.PCW_DDR_HPR_TO_CRITICAL_PRIORITY_LEVEL {15} \
   CONFIG.PCW_DDR_LPR_TO_CRITICAL_PRIORITY_LEVEL {2} \
   CONFIG.PCW_DDR_PERIPHERAL_CLKSRC {DDR PLL} \
   CONFIG.PCW_DDR_PERIPHERAL_DIVISOR0 {2} \
   CONFIG.PCW_DDR_PORT0_HPR_ENABLE {0} \
   CONFIG.PCW_DDR_PORT1_HPR_ENABLE {0} \
   CONFIG.PCW_DDR_PORT2_HPR_ENABLE {0} \
   CONFIG.PCW_DDR_PORT3_HPR_ENABLE {0} \
   CONFIG.PCW_DDR_RAM_HIGHADDR {0x1FFFFFFF} \
   CONFIG.PCW_DDR_WRITE_TO_CRITICAL_PRIORITY_LEVEL {2} \
   CONFIG.PCW_ENET0_ENET0_IO {MIO 16 .. 27} \
   CONFIG.PCW_ENET0_GRP_MDIO_ENABLE {1} \
   CONFIG.PCW_ENET0_GRP_MDIO_IO {EMIO} \
   CONFIG.PCW_ENET0_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_ENET0_PERIPHERAL_DIVISOR0 {8} \
   CONFIG.PCW_ENET0_PERIPHERAL_DIVISOR1 {1} \
   CONFIG.PCW_ENET0_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_ENET0_PERIPHERAL_FREQMHZ {1000 Mbps} \
   CONFIG.PCW_ENET0_RESET_ENABLE {0} \
   CONFIG.PCW_ENET1_GRP_MDIO_ENABLE {0} \
   CONFIG.PCW_ENET1_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_ENET1_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_ENET1_PERIPHERAL_DIVISOR1 {1} \
   CONFIG.PCW_ENET1_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_ENET1_PERIPHERAL_FREQMHZ {1000 Mbps} \
   CONFIG.PCW_ENET1_RESET_ENABLE {0} \
   CONFIG.PCW_ENET_RESET_ENABLE {1} \
   CONFIG.PCW_ENET_RESET_POLARITY {Active Low} \
   CONFIG.PCW_ENET_RESET_SELECT {Share reset pin} \
   CONFIG.PCW_EN_4K_TIMER {0} \
   CONFIG.PCW_EN_CAN0 {0} \
   CONFIG.PCW_EN_CAN1 {0} \
   CONFIG.PCW_EN_CLK0_PORT {1} \
   CONFIG.PCW_EN_CLK1_PORT {1} \
   CONFIG.PCW_EN_CLK2_PORT {0} \
   CONFIG.PCW_EN_CLK3_PORT {0} \
   CONFIG.PCW_EN_DDR {1} \
   CONFIG.PCW_EN_EMIO_CAN0 {0} \
   CONFIG.PCW_EN_EMIO_CAN1 {0} \
   CONFIG.PCW_EN_EMIO_CD_SDIO0 {0} \
   CONFIG.PCW_EN_EMIO_CD_SDIO1 {0} \
   CONFIG.PCW_EN_EMIO_ENET0 {0} \
   CONFIG.PCW_EN_EMIO_ENET1 {0} \
   CONFIG.PCW_EN_EMIO_GPIO {0} \
   CONFIG.PCW_EN_EMIO_I2C0 {0} \
   CONFIG.PCW_EN_EMIO_I2C1 {0} \
   CONFIG.PCW_EN_EMIO_MODEM_UART0 {0} \
   CONFIG.PCW_EN_EMIO_MODEM_UART1 {0} \
   CONFIG.PCW_EN_EMIO_PJTAG {0} \
   CONFIG.PCW_EN_EMIO_SDIO0 {0} \
   CONFIG.PCW_EN_EMIO_SDIO1 {0} \
   CONFIG.PCW_EN_EMIO_SPI0 {1} \
   CONFIG.PCW_EN_EMIO_SPI1 {0} \
   CONFIG.PCW_EN_EMIO_SRAM_INT {0} \
   CONFIG.PCW_EN_EMIO_TRACE {0} \
   CONFIG.PCW_EN_EMIO_TTC0 {1} \
   CONFIG.PCW_EN_EMIO_TTC1 {0} \
   CONFIG.PCW_EN_EMIO_UART0 {0} \
   CONFIG.PCW_EN_EMIO_UART1 {0} \
   CONFIG.PCW_EN_EMIO_WDT {0} \
   CONFIG.PCW_EN_EMIO_WP_SDIO0 {0} \
   CONFIG.PCW_EN_EMIO_WP_SDIO1 {0} \
   CONFIG.PCW_EN_ENET0 {1} \
   CONFIG.PCW_EN_ENET1 {0} \
   CONFIG.PCW_EN_GPIO {1} \
   CONFIG.PCW_EN_I2C0 {1} \
   CONFIG.PCW_EN_I2C1 {0} \
   CONFIG.PCW_EN_MODEM_UART0 {0} \
   CONFIG.PCW_EN_MODEM_UART1 {0} \
   CONFIG.PCW_EN_PJTAG {0} \
   CONFIG.PCW_EN_QSPI {1} \
   CONFIG.PCW_EN_SDIO0 {1} \
   CONFIG.PCW_EN_SDIO1 {0} \
   CONFIG.PCW_EN_SMC {0} \
   CONFIG.PCW_EN_SPI0 {1} \
   CONFIG.PCW_EN_SPI1 {1} \
   CONFIG.PCW_EN_TRACE {0} \
   CONFIG.PCW_EN_TTC0 {1} \
   CONFIG.PCW_EN_TTC1 {0} \
   CONFIG.PCW_EN_UART0 {1} \
   CONFIG.PCW_EN_UART1 {1} \
   CONFIG.PCW_EN_USB0 {1} \
   CONFIG.PCW_EN_USB1 {0} \
   CONFIG.PCW_EN_WDT {0} \
   CONFIG.PCW_FCLK0_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_FCLK0_PERIPHERAL_DIVISOR0 {5} \
   CONFIG.PCW_FCLK0_PERIPHERAL_DIVISOR1 {2} \
   CONFIG.PCW_FCLK1_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_FCLK1_PERIPHERAL_DIVISOR0 {10} \
   CONFIG.PCW_FCLK1_PERIPHERAL_DIVISOR1 {5} \
   CONFIG.PCW_FCLK2_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_FCLK2_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_FCLK2_PERIPHERAL_DIVISOR1 {1} \
   CONFIG.PCW_FCLK3_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_FCLK3_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_FCLK3_PERIPHERAL_DIVISOR1 {1} \
   CONFIG.PCW_FCLK_CLK0_BUF {TRUE} \
   CONFIG.PCW_FCLK_CLK1_BUF {TRUE} \
   CONFIG.PCW_FCLK_CLK2_BUF {FALSE} \
   CONFIG.PCW_FCLK_CLK3_BUF {FALSE} \
   CONFIG.PCW_FPGA0_PERIPHERAL_FREQMHZ {100} \
   CONFIG.PCW_FPGA1_PERIPHERAL_FREQMHZ {20} \
   CONFIG.PCW_FPGA2_PERIPHERAL_FREQMHZ {50} \
   CONFIG.PCW_FPGA3_PERIPHERAL_FREQMHZ {200} \
   CONFIG.PCW_FPGA_FCLK0_ENABLE {1} \
   CONFIG.PCW_FPGA_FCLK1_ENABLE {1} \
   CONFIG.PCW_FPGA_FCLK2_ENABLE {0} \
   CONFIG.PCW_FPGA_FCLK3_ENABLE {0} \
   CONFIG.PCW_GPIO_EMIO_GPIO_ENABLE {0} \
   CONFIG.PCW_GPIO_EMIO_GPIO_WIDTH {64} \
   CONFIG.PCW_GPIO_MIO_GPIO_ENABLE {1} \
   CONFIG.PCW_GPIO_MIO_GPIO_IO {MIO} \
   CONFIG.PCW_GPIO_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_I2C0_GRP_INT_ENABLE {0} \
   CONFIG.PCW_I2C0_I2C0_IO {MIO 50 .. 51} \
   CONFIG.PCW_I2C0_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_I2C0_RESET_ENABLE {0} \
   CONFIG.PCW_I2C1_GRP_INT_ENABLE {0} \
   CONFIG.PCW_I2C1_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_I2C1_RESET_ENABLE {0} \
   CONFIG.PCW_I2C_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_I2C_RESET_ENABLE {1} \
   CONFIG.PCW_I2C_RESET_POLARITY {Active Low} \
   CONFIG.PCW_I2C_RESET_SELECT {Share reset pin} \
   CONFIG.PCW_IMPORT_BOARD_PRESET {cfg/red_pitaya.xml} \
   CONFIG.PCW_IOPLL_CTRL_FBDIV {30} \
   CONFIG.PCW_IO_IO_PLL_FREQMHZ {1000.000} \
   CONFIG.PCW_IRQ_F2P_MODE {DIRECT} \
   CONFIG.PCW_MIO_0_DIRECTION {inout} \
   CONFIG.PCW_MIO_0_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_0_PULLUP {disabled} \
   CONFIG.PCW_MIO_0_SLEW {slow} \
   CONFIG.PCW_MIO_10_DIRECTION {inout} \
   CONFIG.PCW_MIO_10_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_10_PULLUP {enabled} \
   CONFIG.PCW_MIO_10_SLEW {slow} \
   CONFIG.PCW_MIO_11_DIRECTION {inout} \
   CONFIG.PCW_MIO_11_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_11_PULLUP {enabled} \
   CONFIG.PCW_MIO_11_SLEW {slow} \
   CONFIG.PCW_MIO_12_DIRECTION {inout} \
   CONFIG.PCW_MIO_12_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_12_PULLUP {enabled} \
   CONFIG.PCW_MIO_12_SLEW {slow} \
   CONFIG.PCW_MIO_13_DIRECTION {inout} \
   CONFIG.PCW_MIO_13_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_13_PULLUP {enabled} \
   CONFIG.PCW_MIO_13_SLEW {slow} \
   CONFIG.PCW_MIO_14_DIRECTION {in} \
   CONFIG.PCW_MIO_14_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_14_PULLUP {enabled} \
   CONFIG.PCW_MIO_14_SLEW {slow} \
   CONFIG.PCW_MIO_15_DIRECTION {out} \
   CONFIG.PCW_MIO_15_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_15_PULLUP {enabled} \
   CONFIG.PCW_MIO_15_SLEW {slow} \
   CONFIG.PCW_MIO_16_DIRECTION {out} \
   CONFIG.PCW_MIO_16_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_16_PULLUP {disabled} \
   CONFIG.PCW_MIO_16_SLEW {fast} \
   CONFIG.PCW_MIO_17_DIRECTION {out} \
   CONFIG.PCW_MIO_17_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_17_PULLUP {disabled} \
   CONFIG.PCW_MIO_17_SLEW {fast} \
   CONFIG.PCW_MIO_18_DIRECTION {out} \
   CONFIG.PCW_MIO_18_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_18_PULLUP {disabled} \
   CONFIG.PCW_MIO_18_SLEW {fast} \
   CONFIG.PCW_MIO_19_DIRECTION {out} \
   CONFIG.PCW_MIO_19_IOTYPE {out} \
   CONFIG.PCW_MIO_19_PULLUP {disabled} \
   CONFIG.PCW_MIO_19_SLEW {fast} \
   CONFIG.PCW_MIO_1_DIRECTION {out} \
   CONFIG.PCW_MIO_1_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_1_PULLUP {enabled} \
   CONFIG.PCW_MIO_1_SLEW {slow} \
   CONFIG.PCW_MIO_20_DIRECTION {out} \
   CONFIG.PCW_MIO_20_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_20_PULLUP {disabled} \
   CONFIG.PCW_MIO_20_SLEW {fast} \
   CONFIG.PCW_MIO_21_DIRECTION {out} \
   CONFIG.PCW_MIO_21_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_21_PULLUP {disabled} \
   CONFIG.PCW_MIO_21_SLEW {fast} \
   CONFIG.PCW_MIO_22_DIRECTION {in} \
   CONFIG.PCW_MIO_22_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_22_PULLUP {disabled} \
   CONFIG.PCW_MIO_22_SLEW {fast} \
   CONFIG.PCW_MIO_23_DIRECTION {in} \
   CONFIG.PCW_MIO_23_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_23_PULLUP {disabled} \
   CONFIG.PCW_MIO_23_SLEW {fast} \
   CONFIG.PCW_MIO_24_DIRECTION {in} \
   CONFIG.PCW_MIO_24_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_24_PULLUP {disabled} \
   CONFIG.PCW_MIO_24_SLEW {fast} \
   CONFIG.PCW_MIO_25_DIRECTION {in} \
   CONFIG.PCW_MIO_25_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_25_PULLUP {disabled} \
   CONFIG.PCW_MIO_25_SLEW {fast} \
   CONFIG.PCW_MIO_26_DIRECTION {in} \
   CONFIG.PCW_MIO_26_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_26_PULLUP {disabled} \
   CONFIG.PCW_MIO_26_SLEW {fast} \
   CONFIG.PCW_MIO_27_DIRECTION {in} \
   CONFIG.PCW_MIO_27_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_27_PULLUP {disabled} \
   CONFIG.PCW_MIO_27_SLEW {fast} \
   CONFIG.PCW_MIO_28_DIRECTION {inout} \
   CONFIG.PCW_MIO_28_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_28_PULLUP {enabled} \
   CONFIG.PCW_MIO_28_SLEW {fast} \
   CONFIG.PCW_MIO_29_DIRECTION {in} \
   CONFIG.PCW_MIO_29_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_29_PULLUP {enabled} \
   CONFIG.PCW_MIO_29_SLEW {fast} \
   CONFIG.PCW_MIO_2_DIRECTION {inout} \
   CONFIG.PCW_MIO_2_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_2_PULLUP {disabled} \
   CONFIG.PCW_MIO_2_SLEW {slow} \
   CONFIG.PCW_MIO_30_DIRECTION {out} \
   CONFIG.PCW_MIO_30_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_30_PULLUP {enabled} \
   CONFIG.PCW_MIO_30_SLEW {fast} \
   CONFIG.PCW_MIO_31_DIRECTION {in} \
   CONFIG.PCW_MIO_31_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_31_PULLUP {enabled} \
   CONFIG.PCW_MIO_31_SLEW {fast} \
   CONFIG.PCW_MIO_32_DIRECTION {inout} \
   CONFIG.PCW_MIO_32_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_32_PULLUP {enabled} \
   CONFIG.PCW_MIO_32_SLEW {fast} \
   CONFIG.PCW_MIO_33_DIRECTION {inout} \
   CONFIG.PCW_MIO_33_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_33_PULLUP {enabled} \
   CONFIG.PCW_MIO_33_SLEW {fast} \
   CONFIG.PCW_MIO_34_DIRECTION {inout} \
   CONFIG.PCW_MIO_34_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_34_PULLUP {enabled} \
   CONFIG.PCW_MIO_34_SLEW {fast} \
   CONFIG.PCW_MIO_35_DIRECTION {inout} \
   CONFIG.PCW_MIO_35_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_35_PULLUP {enabled} \
   CONFIG.PCW_MIO_35_SLEW {fast} \
   CONFIG.PCW_MIO_36_DIRECTION {in} \
   CONFIG.PCW_MIO_36_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_36_PULLUP {enabled} \
   CONFIG.PCW_MIO_36_SLEW {fast} \
   CONFIG.PCW_MIO_37_DIRECTION {inout} \
   CONFIG.PCW_MIO_37_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_37_PULLUP {enabled} \
   CONFIG.PCW_MIO_37_SLEW {fast} \
   CONFIG.PCW_MIO_38_DIRECTION {inout} \
   CONFIG.PCW_MIO_38_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_38_PULLUP {enabled} \
   CONFIG.PCW_MIO_38_SLEW {fast} \
   CONFIG.PCW_MIO_39_DIRECTION {inout} \
   CONFIG.PCW_MIO_39_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_39_PULLUP {enabled} \
   CONFIG.PCW_MIO_39_SLEW {fast} \
   CONFIG.PCW_MIO_3_DIRECTION {inout} \
   CONFIG.PCW_MIO_3_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_3_PULLUP {disabled} \
   CONFIG.PCW_MIO_3_SLEW {slow} \
   CONFIG.PCW_MIO_40_DIRECTION {inout} \
   CONFIG.PCW_MIO_40_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_40_PULLUP {enabled} \
   CONFIG.PCW_MIO_40_SLEW {slow} \
   CONFIG.PCW_MIO_41_DIRECTION {inout} \
   CONFIG.PCW_MIO_41_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_41_PULLUP {enabled} \
   CONFIG.PCW_MIO_41_SLEW {slow} \
   CONFIG.PCW_MIO_42_DIRECTION {inout} \
   CONFIG.PCW_MIO_42_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_42_PULLUP {enabled} \
   CONFIG.PCW_MIO_42_SLEW {slow} \
   CONFIG.PCW_MIO_43_DIRECTION {inout} \
   CONFIG.PCW_MIO_43_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_43_PULLUP {enabled} \
   CONFIG.PCW_MIO_43_SLEW {slow} \
   CONFIG.PCW_MIO_44_DIRECTION {inout} \
   CONFIG.PCW_MIO_44_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_44_PULLUP {enabled} \
   CONFIG.PCW_MIO_44_SLEW {slow} \
   CONFIG.PCW_MIO_45_DIRECTION {inout} \
   CONFIG.PCW_MIO_45_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_45_PULLUP {enabled} \
   CONFIG.PCW_MIO_45_SLEW {slow} \
   CONFIG.PCW_MIO_46_DIRECTION {in} \
   CONFIG.PCW_MIO_46_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_46_PULLUP {enabled} \
   CONFIG.PCW_MIO_46_SLEW {slow} \
   CONFIG.PCW_MIO_47_DIRECTION {in} \
   CONFIG.PCW_MIO_47_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_47_PULLUP {enabled} \
   CONFIG.PCW_MIO_47_SLEW {slow} \
   CONFIG.PCW_MIO_48_DIRECTION {out} \
   CONFIG.PCW_MIO_48_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_48_PULLUP {enabled} \
   CONFIG.PCW_MIO_48_SLEW {slow} \
   CONFIG.PCW_MIO_49_DIRECTION {inout} \
   CONFIG.PCW_MIO_49_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_49_PULLUP {enabled} \
   CONFIG.PCW_MIO_49_SLEW {slow} \
   CONFIG.PCW_MIO_4_DIRECTION {inout} \
   CONFIG.PCW_MIO_4_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_4_PULLUP {disabled} \
   CONFIG.PCW_MIO_4_SLEW {slow} \
   CONFIG.PCW_MIO_50_DIRECTION {inout} \
   CONFIG.PCW_MIO_50_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_50_PULLUP {enabled} \
   CONFIG.PCW_MIO_50_SLEW {slow} \
   CONFIG.PCW_MIO_51_DIRECTION {inout} \
   CONFIG.PCW_MIO_51_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_51_PULLUP {enabled} \
   CONFIG.PCW_MIO_51_SLEW {slow} \
   CONFIG.PCW_MIO_52_DIRECTION {inout} \
   CONFIG.PCW_MIO_52_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_52_PULLUP {enabled} \
   CONFIG.PCW_MIO_52_SLEW {slow} \
   CONFIG.PCW_MIO_53_DIRECTION {inout} \
   CONFIG.PCW_MIO_53_IOTYPE {LVCMOS 2.5V} \
   CONFIG.PCW_MIO_53_PULLUP {enabled} \
   CONFIG.PCW_MIO_53_SLEW {slow} \
   CONFIG.PCW_MIO_5_DIRECTION {inout} \
   CONFIG.PCW_MIO_5_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_5_PULLUP {disabled} \
   CONFIG.PCW_MIO_5_SLEW {slow} \
   CONFIG.PCW_MIO_6_DIRECTION {out} \
   CONFIG.PCW_MIO_6_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_6_PULLUP {disabled} \
   CONFIG.PCW_MIO_6_SLEW {slow} \
   CONFIG.PCW_MIO_7_DIRECTION {out} \
   CONFIG.PCW_MIO_7_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_7_PULLUP {disabled} \
   CONFIG.PCW_MIO_7_SLEW {slow} \
   CONFIG.PCW_MIO_8_DIRECTION {out} \
   CONFIG.PCW_MIO_8_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_8_PULLUP {disabled} \
   CONFIG.PCW_MIO_8_SLEW {slow} \
   CONFIG.PCW_MIO_9_DIRECTION {in} \
   CONFIG.PCW_MIO_9_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_9_PULLUP {enabled} \
   CONFIG.PCW_MIO_9_SLEW {slow} \
   CONFIG.PCW_MIO_TREE_PERIPHERALS {GPIO#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#GPIO#UART 1#UART 1#SPI 1#SPI 1#SPI 1#SPI 1#UART 0#UART 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#SD 0#SD 0#SD 0#SD 0#SD 0#SD 0#SD 0#SD 0#USB Reset#GPIO#I2C 0#I2C 0#GPIO#GPIO} \
   CONFIG.PCW_MIO_TREE_SIGNALS {gpio[0]#qspi0_ss_b#qspi0_io[0]#qspi0_io[1]#qspi0_io[2]#qspi0_io[3]/HOLD_B#qspi0_sclk#gpio[7]#tx#rx#mosi#miso#sclk#ss[0]#rx#tx#tx_clk#txd[0]#txd[1]#txd[2]#txd[3]#tx_ctl#rx_clk#rxd[0]#rxd[1]#rxd[2]#rxd[3]#rx_ctl#data[4]#dir#stp#nxt#data[0]#data[1]#data[2]#data[3]#clk#data[5]#data[6]#data[7]#clk#cmd#data[0]#data[1]#data[2]#data[3]#cd#wp#reset#gpio[49]#scl#sda#gpio[52]#gpio[53]} \
   CONFIG.PCW_NAND_CYCLES_T_AR {1} \
   CONFIG.PCW_NAND_CYCLES_T_CLR {1} \
   CONFIG.PCW_NAND_CYCLES_T_RC {11} \
   CONFIG.PCW_NAND_CYCLES_T_REA {1} \
   CONFIG.PCW_NAND_CYCLES_T_RR {1} \
   CONFIG.PCW_NAND_CYCLES_T_WC {11} \
   CONFIG.PCW_NAND_CYCLES_T_WP {1} \
   CONFIG.PCW_NAND_GRP_D8_ENABLE {0} \
   CONFIG.PCW_NAND_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_NOR_CS0_T_CEOE {1} \
   CONFIG.PCW_NOR_CS0_T_PC {1} \
   CONFIG.PCW_NOR_CS0_T_RC {11} \
   CONFIG.PCW_NOR_CS0_T_TR {1} \
   CONFIG.PCW_NOR_CS0_T_WC {11} \
   CONFIG.PCW_NOR_CS0_T_WP {1} \
   CONFIG.PCW_NOR_CS0_WE_TIME {0} \
   CONFIG.PCW_NOR_CS1_T_CEOE {1} \
   CONFIG.PCW_NOR_CS1_T_PC {1} \
   CONFIG.PCW_NOR_CS1_T_RC {11} \
   CONFIG.PCW_NOR_CS1_T_TR {1} \
   CONFIG.PCW_NOR_CS1_T_WC {11} \
   CONFIG.PCW_NOR_CS1_T_WP {1} \
   CONFIG.PCW_NOR_CS1_WE_TIME {0} \
   CONFIG.PCW_NOR_GRP_A25_ENABLE {0} \
   CONFIG.PCW_NOR_GRP_CS0_ENABLE {0} \
   CONFIG.PCW_NOR_GRP_CS1_ENABLE {0} \
   CONFIG.PCW_NOR_GRP_SRAM_CS0_ENABLE {0} \
   CONFIG.PCW_NOR_GRP_SRAM_CS1_ENABLE {0} \
   CONFIG.PCW_NOR_GRP_SRAM_INT_ENABLE {0} \
   CONFIG.PCW_NOR_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_NOR_SRAM_CS0_T_CEOE {1} \
   CONFIG.PCW_NOR_SRAM_CS0_T_PC {1} \
   CONFIG.PCW_NOR_SRAM_CS0_T_RC {11} \
   CONFIG.PCW_NOR_SRAM_CS0_T_TR {1} \
   CONFIG.PCW_NOR_SRAM_CS0_T_WC {11} \
   CONFIG.PCW_NOR_SRAM_CS0_T_WP {1} \
   CONFIG.PCW_NOR_SRAM_CS0_WE_TIME {0} \
   CONFIG.PCW_NOR_SRAM_CS1_T_CEOE {1} \
   CONFIG.PCW_NOR_SRAM_CS1_T_PC {1} \
   CONFIG.PCW_NOR_SRAM_CS1_T_RC {11} \
   CONFIG.PCW_NOR_SRAM_CS1_T_TR {1} \
   CONFIG.PCW_NOR_SRAM_CS1_T_WC {11} \
   CONFIG.PCW_NOR_SRAM_CS1_T_WP {1} \
   CONFIG.PCW_NOR_SRAM_CS1_WE_TIME {0} \
   CONFIG.PCW_OVERRIDE_BASIC_CLOCK {0} \
   CONFIG.PCW_PACKAGE_DDR_BOARD_DELAY0 {0.080} \
   CONFIG.PCW_PACKAGE_DDR_BOARD_DELAY1 {0.063} \
   CONFIG.PCW_PACKAGE_DDR_BOARD_DELAY2 {0.057} \
   CONFIG.PCW_PACKAGE_DDR_BOARD_DELAY3 {0.068} \
   CONFIG.PCW_PACKAGE_DDR_DQS_TO_CLK_DELAY_0 {-0.047} \
   CONFIG.PCW_PACKAGE_DDR_DQS_TO_CLK_DELAY_1 {-0.025} \
   CONFIG.PCW_PACKAGE_DDR_DQS_TO_CLK_DELAY_2 {-0.006} \
   CONFIG.PCW_PACKAGE_DDR_DQS_TO_CLK_DELAY_3 {-0.017} \
   CONFIG.PCW_PCAP_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_PCAP_PERIPHERAL_DIVISOR0 {5} \
   CONFIG.PCW_PCAP_PERIPHERAL_FREQMHZ {200} \
   CONFIG.PCW_PJTAG_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_PLL_BYPASSMODE_ENABLE {0} \
   CONFIG.PCW_PRESET_BANK0_VOLTAGE {LVCMOS 3.3V} \
   CONFIG.PCW_PRESET_BANK1_VOLTAGE {LVCMOS 2.5V} \
   CONFIG.PCW_QSPI_GRP_FBCLK_ENABLE {0} \
   CONFIG.PCW_QSPI_GRP_IO1_ENABLE {0} \
   CONFIG.PCW_QSPI_GRP_SINGLE_SS_ENABLE {1} \
   CONFIG.PCW_QSPI_GRP_SINGLE_SS_IO {MIO 1 .. 6} \
   CONFIG.PCW_QSPI_GRP_SS1_ENABLE {0} \
   CONFIG.PCW_QSPI_INTERNAL_HIGHADDRESS {0xFCFFFFFF} \
   CONFIG.PCW_QSPI_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_QSPI_PERIPHERAL_DIVISOR0 {8} \
   CONFIG.PCW_QSPI_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_QSPI_PERIPHERAL_FREQMHZ {125} \
   CONFIG.PCW_QSPI_QSPI_IO {MIO 1 .. 6} \
   CONFIG.PCW_SD0_GRP_CD_ENABLE {1} \
   CONFIG.PCW_SD0_GRP_CD_IO {MIO 46} \
   CONFIG.PCW_SD0_GRP_POW_ENABLE {0} \
   CONFIG.PCW_SD0_GRP_WP_ENABLE {1} \
   CONFIG.PCW_SD0_GRP_WP_IO {MIO 47} \
   CONFIG.PCW_SD0_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_SD0_SD0_IO {MIO 40 .. 45} \
   CONFIG.PCW_SD1_GRP_CD_ENABLE {0} \
   CONFIG.PCW_SD1_GRP_POW_ENABLE {0} \
   CONFIG.PCW_SD1_GRP_WP_ENABLE {0} \
   CONFIG.PCW_SD1_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_SDIO_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_SDIO_PERIPHERAL_DIVISOR0 {10} \
   CONFIG.PCW_SDIO_PERIPHERAL_FREQMHZ {100} \
   CONFIG.PCW_SDIO_PERIPHERAL_VALID {1} \
   CONFIG.PCW_SINGLE_QSPI_DATA_MODE {x4} \
   CONFIG.PCW_SMC_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_SMC_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_SMC_PERIPHERAL_FREQMHZ {100} \
   CONFIG.PCW_SMC_PERIPHERAL_VALID {0} \
   CONFIG.PCW_SPI0_GRP_SS0_ENABLE {1} \
   CONFIG.PCW_SPI0_GRP_SS0_IO {EMIO} \
   CONFIG.PCW_SPI0_GRP_SS1_ENABLE {1} \
   CONFIG.PCW_SPI0_GRP_SS1_IO {EMIO} \
   CONFIG.PCW_SPI0_GRP_SS2_ENABLE {1} \
   CONFIG.PCW_SPI0_GRP_SS2_IO {EMIO} \
   CONFIG.PCW_SPI0_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_SPI0_SPI0_IO {EMIO} \
   CONFIG.PCW_SPI1_GRP_SS0_ENABLE {1} \
   CONFIG.PCW_SPI1_GRP_SS0_IO {MIO 13} \
   CONFIG.PCW_SPI1_GRP_SS1_ENABLE {0} \
   CONFIG.PCW_SPI1_GRP_SS1_IO {<Select>} \
   CONFIG.PCW_SPI1_GRP_SS2_ENABLE {0} \
   CONFIG.PCW_SPI1_GRP_SS2_IO {<Select>} \
   CONFIG.PCW_SPI1_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_SPI1_SPI1_IO {MIO 10 .. 15} \
   CONFIG.PCW_SPI_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_SPI_PERIPHERAL_DIVISOR0 {6} \
   CONFIG.PCW_SPI_PERIPHERAL_FREQMHZ {166.666666} \
   CONFIG.PCW_SPI_PERIPHERAL_VALID {1} \
   CONFIG.PCW_S_AXI_HP0_DATA_WIDTH {64} \
   CONFIG.PCW_S_AXI_HP1_DATA_WIDTH {64} \
   CONFIG.PCW_S_AXI_HP2_DATA_WIDTH {64} \
   CONFIG.PCW_S_AXI_HP3_DATA_WIDTH {64} \
   CONFIG.PCW_TPIU_PERIPHERAL_CLKSRC {External} \
   CONFIG.PCW_TPIU_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_TPIU_PERIPHERAL_FREQMHZ {200} \
   CONFIG.PCW_TRACE_GRP_16BIT_ENABLE {0} \
   CONFIG.PCW_TRACE_GRP_2BIT_ENABLE {0} \
   CONFIG.PCW_TRACE_GRP_32BIT_ENABLE {0} \
   CONFIG.PCW_TRACE_GRP_4BIT_ENABLE {0} \
   CONFIG.PCW_TRACE_GRP_8BIT_ENABLE {0} \
   CONFIG.PCW_TRACE_INTERNAL_WIDTH {2} \
   CONFIG.PCW_TRACE_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_TTC0_CLK0_PERIPHERAL_CLKSRC {CPU_1X} \
   CONFIG.PCW_TTC0_CLK0_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_TTC0_CLK0_PERIPHERAL_FREQMHZ {133.333333} \
   CONFIG.PCW_TTC0_CLK1_PERIPHERAL_CLKSRC {CPU_1X} \
   CONFIG.PCW_TTC0_CLK1_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_TTC0_CLK1_PERIPHERAL_FREQMHZ {133.333333} \
   CONFIG.PCW_TTC0_CLK2_PERIPHERAL_CLKSRC {CPU_1X} \
   CONFIG.PCW_TTC0_CLK2_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_TTC0_CLK2_PERIPHERAL_FREQMHZ {133.333333} \
   CONFIG.PCW_TTC0_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_TTC0_TTC0_IO {EMIO} \
   CONFIG.PCW_TTC1_CLK0_PERIPHERAL_CLKSRC {CPU_1X} \
   CONFIG.PCW_TTC1_CLK0_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_TTC1_CLK0_PERIPHERAL_FREQMHZ {133.333333} \
   CONFIG.PCW_TTC1_CLK1_PERIPHERAL_CLKSRC {CPU_1X} \
   CONFIG.PCW_TTC1_CLK1_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_TTC1_CLK1_PERIPHERAL_FREQMHZ {133.333333} \
   CONFIG.PCW_TTC1_CLK2_PERIPHERAL_CLKSRC {CPU_1X} \
   CONFIG.PCW_TTC1_CLK2_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_TTC1_CLK2_PERIPHERAL_FREQMHZ {133.333333} \
   CONFIG.PCW_TTC1_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_TTC_PERIPHERAL_FREQMHZ {50} \
   CONFIG.PCW_UART0_BAUD_RATE {115200} \
   CONFIG.PCW_UART0_GRP_FULL_ENABLE {0} \
   CONFIG.PCW_UART0_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_UART0_UART0_IO {MIO 14 .. 15} \
   CONFIG.PCW_UART1_BAUD_RATE {115200} \
   CONFIG.PCW_UART1_GRP_FULL_ENABLE {0} \
   CONFIG.PCW_UART1_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_UART1_UART1_IO {MIO 8 .. 9} \
   CONFIG.PCW_UART_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_UART_PERIPHERAL_DIVISOR0 {10} \
   CONFIG.PCW_UART_PERIPHERAL_FREQMHZ {100} \
   CONFIG.PCW_UART_PERIPHERAL_VALID {1} \
   CONFIG.PCW_UIPARAM_ACT_DDR_FREQ_MHZ {533.333374} \
   CONFIG.PCW_UIPARAM_DDR_ADV_ENABLE {0} \
   CONFIG.PCW_UIPARAM_DDR_AL {0} \
   CONFIG.PCW_UIPARAM_DDR_BANK_ADDR_COUNT {3} \
   CONFIG.PCW_UIPARAM_DDR_BL {8} \
   CONFIG.PCW_UIPARAM_DDR_BOARD_DELAY0 {0.25} \
   CONFIG.PCW_UIPARAM_DDR_BOARD_DELAY1 {0.25} \
   CONFIG.PCW_UIPARAM_DDR_BOARD_DELAY2 {0.25} \
   CONFIG.PCW_UIPARAM_DDR_BOARD_DELAY3 {0.25} \
   CONFIG.PCW_UIPARAM_DDR_BUS_WIDTH {16 Bit} \
   CONFIG.PCW_UIPARAM_DDR_CL {7} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_0_LENGTH_MM {0} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_0_PACKAGE_LENGTH {54.563} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_0_PROPOGATION_DELAY {160} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_1_LENGTH_MM {0} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_1_PACKAGE_LENGTH {54.563} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_1_PROPOGATION_DELAY {160} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_2_LENGTH_MM {0} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_2_PACKAGE_LENGTH {54.563} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_2_PROPOGATION_DELAY {160} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_3_LENGTH_MM {0} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_3_PACKAGE_LENGTH {54.563} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_3_PROPOGATION_DELAY {160} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_STOP_EN {0} \
   CONFIG.PCW_UIPARAM_DDR_COL_ADDR_COUNT {10} \
   CONFIG.PCW_UIPARAM_DDR_CWL {6} \
   CONFIG.PCW_UIPARAM_DDR_DEVICE_CAPACITY {4096 MBits} \
   CONFIG.PCW_UIPARAM_DDR_DQS_0_LENGTH_MM {0} \
   CONFIG.PCW_UIPARAM_DDR_DQS_0_PACKAGE_LENGTH {101.239} \
   CONFIG.PCW_UIPARAM_DDR_DQS_0_PROPOGATION_DELAY {160} \
   CONFIG.PCW_UIPARAM_DDR_DQS_1_LENGTH_MM {0} \
   CONFIG.PCW_UIPARAM_DDR_DQS_1_PACKAGE_LENGTH {79.5025} \
   CONFIG.PCW_UIPARAM_DDR_DQS_1_PROPOGATION_DELAY {160} \
   CONFIG.PCW_UIPARAM_DDR_DQS_2_LENGTH_MM {0} \
   CONFIG.PCW_UIPARAM_DDR_DQS_2_PACKAGE_LENGTH {60.536} \
   CONFIG.PCW_UIPARAM_DDR_DQS_2_PROPOGATION_DELAY {160} \
   CONFIG.PCW_UIPARAM_DDR_DQS_3_LENGTH_MM {0} \
   CONFIG.PCW_UIPARAM_DDR_DQS_3_PACKAGE_LENGTH {71.7715} \
   CONFIG.PCW_UIPARAM_DDR_DQS_3_PROPOGATION_DELAY {160} \
   CONFIG.PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_0 {0.0} \
   CONFIG.PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_1 {0.0} \
   CONFIG.PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_2 {0.0} \
   CONFIG.PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_3 {0.0} \
   CONFIG.PCW_UIPARAM_DDR_DQ_0_LENGTH_MM {0} \
   CONFIG.PCW_UIPARAM_DDR_DQ_0_PACKAGE_LENGTH {104.5365} \
   CONFIG.PCW_UIPARAM_DDR_DQ_0_PROPOGATION_DELAY {160} \
   CONFIG.PCW_UIPARAM_DDR_DQ_1_LENGTH_MM {0} \
   CONFIG.PCW_UIPARAM_DDR_DQ_1_PACKAGE_LENGTH {70.676} \
   CONFIG.PCW_UIPARAM_DDR_DQ_1_PROPOGATION_DELAY {160} \
   CONFIG.PCW_UIPARAM_DDR_DQ_2_LENGTH_MM {0} \
   CONFIG.PCW_UIPARAM_DDR_DQ_2_PACKAGE_LENGTH {59.1615} \
   CONFIG.PCW_UIPARAM_DDR_DQ_2_PROPOGATION_DELAY {160} \
   CONFIG.PCW_UIPARAM_DDR_DQ_3_LENGTH_MM {0} \
   CONFIG.PCW_UIPARAM_DDR_DQ_3_PACKAGE_LENGTH {81.319} \
   CONFIG.PCW_UIPARAM_DDR_DQ_3_PROPOGATION_DELAY {160} \
   CONFIG.PCW_UIPARAM_DDR_DRAM_WIDTH {16 Bits} \
   CONFIG.PCW_UIPARAM_DDR_ECC {Disabled} \
   CONFIG.PCW_UIPARAM_DDR_ENABLE {1} \
   CONFIG.PCW_UIPARAM_DDR_FREQ_MHZ {533.333333} \
   CONFIG.PCW_UIPARAM_DDR_HIGH_TEMP {Normal (0-85)} \
   CONFIG.PCW_UIPARAM_DDR_MEMORY_TYPE {DDR 3} \
   CONFIG.PCW_UIPARAM_DDR_PARTNO {MT41J256M16 RE-125} \
   CONFIG.PCW_UIPARAM_DDR_ROW_ADDR_COUNT {15} \
   CONFIG.PCW_UIPARAM_DDR_SPEED_BIN {DDR3_1066F} \
   CONFIG.PCW_UIPARAM_DDR_TRAIN_DATA_EYE {1} \
   CONFIG.PCW_UIPARAM_DDR_TRAIN_READ_GATE {1} \
   CONFIG.PCW_UIPARAM_DDR_TRAIN_WRITE_LEVEL {1} \
   CONFIG.PCW_UIPARAM_DDR_T_FAW {40.0} \
   CONFIG.PCW_UIPARAM_DDR_T_RAS_MIN {35.0} \
   CONFIG.PCW_UIPARAM_DDR_T_RC {48.91} \
   CONFIG.PCW_UIPARAM_DDR_T_RCD {7} \
   CONFIG.PCW_UIPARAM_DDR_T_RP {7} \
   CONFIG.PCW_UIPARAM_DDR_USE_INTERNAL_VREF {0} \
   CONFIG.PCW_USB0_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_USB0_PERIPHERAL_FREQMHZ {60} \
   CONFIG.PCW_USB0_RESET_ENABLE {1} \
   CONFIG.PCW_USB0_RESET_IO {MIO 48} \
   CONFIG.PCW_USB0_USB0_IO {MIO 28 .. 39} \
   CONFIG.PCW_USB1_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_USB1_PERIPHERAL_FREQMHZ {60} \
   CONFIG.PCW_USB1_RESET_ENABLE {0} \
   CONFIG.PCW_USB_RESET_ENABLE {1} \
   CONFIG.PCW_USB_RESET_POLARITY {Active Low} \
   CONFIG.PCW_USB_RESET_SELECT {Share reset pin} \
   CONFIG.PCW_USE_AXI_NONSECURE {0} \
   CONFIG.PCW_USE_CROSS_TRIGGER {0} \
   CONFIG.PCW_USE_DMA0 {0} \
   CONFIG.PCW_USE_S_AXI_ACP {1} \
   CONFIG.PCW_USE_S_AXI_HP0 {0} \
   CONFIG.PCW_WDT_PERIPHERAL_CLKSRC {CPU_1X} \
   CONFIG.PCW_WDT_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_WDT_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_WDT_PERIPHERAL_FREQMHZ {133.333333} \
 ] $processing_system7_0

  # Create instance: rst_ps7_0_125M, and set properties
  set rst_ps7_0_125M [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 rst_ps7_0_125M ]

  # Create interface connections
  connect_bd_intf_net -intf_net Conn1 [get_bd_intf_pins M08_AXI] [get_bd_intf_pins axi_interconnect_0/M08_AXI]
  connect_bd_intf_net -intf_net Conn2 [get_bd_intf_pins M09_AXI] [get_bd_intf_pins axi_interconnect_0/M09_AXI]
  connect_bd_intf_net -intf_net Conn3 [get_bd_intf_pins M10_AXI] [get_bd_intf_pins axi_interconnect_0/M10_AXI]
  connect_bd_intf_net -intf_net axi_bram_reader_0_BRAM_PORTA [get_bd_intf_pins BRAM_PORTA] [get_bd_intf_pins axi_bram_reader_0/BRAM_PORTA]
  connect_bd_intf_net -intf_net axi_interconnect_0_M00_AXI [get_bd_intf_pins axi_cfg_register_0/S_AXI] [get_bd_intf_pins axi_interconnect_0/M00_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M01_AXI [get_bd_intf_pins axi_gpio_0/S_AXI] [get_bd_intf_pins axi_interconnect_0/M01_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M02_AXI [get_bd_intf_pins axi_gpio_1/S_AXI] [get_bd_intf_pins axi_interconnect_0/M02_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M03_AXI [get_bd_intf_pins axi_gpio_2/S_AXI] [get_bd_intf_pins axi_interconnect_0/M03_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M04_AXI [get_bd_intf_pins axi_gpio_3/S_AXI] [get_bd_intf_pins axi_interconnect_0/M04_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M05_AXI [get_bd_intf_pins axi_gpio_4/S_AXI] [get_bd_intf_pins axi_interconnect_0/M05_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M06_AXI [get_bd_intf_pins axi_bram_reader_0/S_AXI] [get_bd_intf_pins axi_interconnect_0/M06_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M07_AXI [get_bd_intf_pins axi_gpio_5/S_AXI] [get_bd_intf_pins axi_interconnect_0/M07_AXI]
  connect_bd_intf_net -intf_net processing_system7_0_DDR [get_bd_intf_pins DDR] [get_bd_intf_pins processing_system7_0/DDR]
  connect_bd_intf_net -intf_net processing_system7_0_FIXED_IO [get_bd_intf_pins FIXED_IO] [get_bd_intf_pins processing_system7_0/FIXED_IO]
  connect_bd_intf_net -intf_net processing_system7_0_M_AXI_GP0 [get_bd_intf_pins axi_interconnect_0/S00_AXI] [get_bd_intf_pins processing_system7_0/M_AXI_GP0]

  # Create port connections
  connect_bd_net -net Net [get_bd_pins FCLK_CLK0] [get_bd_pins axi_bram_reader_0/s00_axi_aclk] [get_bd_pins axi_cfg_register_0/aclk] [get_bd_pins axi_gpio_0/s_axi_aclk] [get_bd_pins axi_gpio_1/s_axi_aclk] [get_bd_pins axi_gpio_2/s_axi_aclk] [get_bd_pins axi_gpio_3/s_axi_aclk] [get_bd_pins axi_gpio_4/s_axi_aclk] [get_bd_pins axi_gpio_5/s_axi_aclk] [get_bd_pins axi_interconnect_0/ACLK] [get_bd_pins axi_interconnect_0/M00_ACLK] [get_bd_pins axi_interconnect_0/M01_ACLK] [get_bd_pins axi_interconnect_0/M02_ACLK] [get_bd_pins axi_interconnect_0/M03_ACLK] [get_bd_pins axi_interconnect_0/M04_ACLK] [get_bd_pins axi_interconnect_0/M05_ACLK] [get_bd_pins axi_interconnect_0/M06_ACLK] [get_bd_pins axi_interconnect_0/M07_ACLK] [get_bd_pins axi_interconnect_0/M08_ACLK] [get_bd_pins axi_interconnect_0/M09_ACLK] [get_bd_pins axi_interconnect_0/M10_ACLK] [get_bd_pins axi_interconnect_0/S00_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins processing_system7_0/M_AXI_GP0_ACLK] [get_bd_pins processing_system7_0/S_AXI_ACP_ACLK] [get_bd_pins rst_ps7_0_125M/slowest_sync_clk]
  connect_bd_net -net SPI0_MISO_I_1 [get_bd_pins SPI0_MISO_I] [get_bd_pins processing_system7_0/SPI0_MISO_I]
  connect_bd_net -net SPI0_SCLK_I_1 [get_bd_pins SPI0_SCLK_I] [get_bd_pins processing_system7_0/SPI0_SCLK_I]
  connect_bd_net -net axi_cfg_register_0_cfg_data [get_bd_pins cfg_data] [get_bd_pins axi_cfg_register_0/cfg_data]
  connect_bd_net -net gpio2_io_i1_1 [get_bd_pins gpio_io_1_x4] [get_bd_pins axi_gpio_1/gpio2_io_i]
  connect_bd_net -net gpio2_io_i3_1 [get_bd_pins gpio_io_3_x8] [get_bd_pins axi_gpio_3/gpio2_io_i]
  connect_bd_net -net gpio2_io_i4_1 [get_bd_pins gpio_io_2_x6] [get_bd_pins axi_gpio_2/gpio2_io_i]
  connect_bd_net -net gpio2_io_i_1 [get_bd_pins gpio_io_0_x2] [get_bd_pins axi_gpio_0/gpio2_io_i]
  connect_bd_net -net gpio2_io_i_2 [get_bd_pins gpio_io_4_x10] [get_bd_pins axi_gpio_4/gpio2_io_i]
  connect_bd_net -net gpio2_io_i_3 [get_bd_pins gpio_io_5_x12] [get_bd_pins axi_gpio_5/gpio2_io_i]
  connect_bd_net -net gpio_io_i1_1 [get_bd_pins gpio_io_1_x3] [get_bd_pins axi_gpio_1/gpio_io_i]
  connect_bd_net -net gpio_io_i3_1 [get_bd_pins gpio_io_3_x7] [get_bd_pins axi_gpio_3/gpio_io_i]
  connect_bd_net -net gpio_io_i4_1 [get_bd_pins gpio_io_2_x5] [get_bd_pins axi_gpio_2/gpio_io_i]
  connect_bd_net -net gpio_io_i_1 [get_bd_pins gpio_io_0_x1] [get_bd_pins axi_gpio_0/gpio_io_i]
  connect_bd_net -net gpio_io_i_2 [get_bd_pins gpio_io_4_x9] [get_bd_pins axi_gpio_4/gpio_io_i]
  connect_bd_net -net gpio_io_i_3 [get_bd_pins gpio_io_5_x11] [get_bd_pins axi_gpio_5/gpio_io_i]
  connect_bd_net -net proc_sys_reset_0_interconnect_aresetn [get_bd_pins interconnect_aresetn1] [get_bd_pins proc_sys_reset_0/interconnect_aresetn]
  connect_bd_net -net proc_sys_reset_0_peripheral_aresetn [get_bd_pins peripheral_aresetn] [get_bd_pins proc_sys_reset_0/peripheral_aresetn]
  connect_bd_net -net processing_system7_0_FCLK_RESET0_N [get_bd_pins proc_sys_reset_0/ext_reset_in] [get_bd_pins processing_system7_0/FCLK_RESET0_N] [get_bd_pins rst_ps7_0_125M/ext_reset_in]
  connect_bd_net -net processing_system7_0_SPI0_MOSI_O [get_bd_pins SPI0_MOSI_O] [get_bd_pins processing_system7_0/SPI0_MOSI_O]
  connect_bd_net -net processing_system7_0_SPI0_SCLK_O [get_bd_pins SPI0_SCLK_O] [get_bd_pins processing_system7_0/SPI0_SCLK_O]
  connect_bd_net -net processing_system7_0_SPI0_SS_O [get_bd_pins SPI0_SS_O] [get_bd_pins processing_system7_0/SPI0_SS_O]
  connect_bd_net -net rst_ps7_0_125M_interconnect_aresetn [get_bd_pins axi_interconnect_0/ARESETN] [get_bd_pins rst_ps7_0_125M/interconnect_aresetn]
  connect_bd_net -net rst_ps7_0_125M_peripheral_aresetn [get_bd_pins peripheral_aresetn1] [get_bd_pins axi_bram_reader_0/s00_axi_aresetn] [get_bd_pins axi_cfg_register_0/aresetn] [get_bd_pins axi_gpio_0/s_axi_aresetn] [get_bd_pins axi_gpio_1/s_axi_aresetn] [get_bd_pins axi_gpio_2/s_axi_aresetn] [get_bd_pins axi_gpio_3/s_axi_aresetn] [get_bd_pins axi_gpio_4/s_axi_aresetn] [get_bd_pins axi_gpio_5/s_axi_aresetn] [get_bd_pins axi_interconnect_0/M00_ARESETN] [get_bd_pins axi_interconnect_0/M01_ARESETN] [get_bd_pins axi_interconnect_0/M02_ARESETN] [get_bd_pins axi_interconnect_0/M03_ARESETN] [get_bd_pins axi_interconnect_0/M04_ARESETN] [get_bd_pins axi_interconnect_0/M05_ARESETN] [get_bd_pins axi_interconnect_0/M06_ARESETN] [get_bd_pins axi_interconnect_0/M07_ARESETN] [get_bd_pins axi_interconnect_0/M08_ARESETN] [get_bd_pins axi_interconnect_0/M09_ARESETN] [get_bd_pins axi_interconnect_0/M10_ARESETN] [get_bd_pins axi_interconnect_0/S00_ARESETN] [get_bd_pins rst_ps7_0_125M/peripheral_aresetn]
  connect_bd_net -net slowest_sync_clk_1 [get_bd_pins slowest_sync_clk] [get_bd_pins proc_sys_reset_0/slowest_sync_clk]

  # Restore current instance
  current_bd_instance $oldCurInst
}


# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  variable script_folder
  variable design_name

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports
  set DDR [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:ddrx_rtl:1.0 DDR ]

  set FIXED_IO [ create_bd_intf_port -mode Master -vlnv xilinx.com:display_processing_system7:fixedio_rtl:1.0 FIXED_IO ]

  set Vaux0 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux0 ]

  set Vaux1 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux1 ]

  set Vaux8 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux8 ]

  set Vaux9 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux9 ]

  set Vp_Vn [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vp_Vn ]

  set gpio_rtl_0 [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:gpio_rtl:1.0 gpio_rtl_0 ]

  set spi_rtl_0 [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:spi_rtl:1.0 spi_rtl_0 ]


  # Create ports
  set adc_clk_n_i [ create_bd_port -dir I adc_clk_n_i ]
  set adc_clk_p_i [ create_bd_port -dir I adc_clk_p_i ]
  set adc_csn_o [ create_bd_port -dir O adc_csn_o ]
  set adc_dat_a_i [ create_bd_port -dir I -from 13 -to 0 adc_dat_a_i ]
  set adc_dat_b_i [ create_bd_port -dir I -from 13 -to 0 adc_dat_b_i ]
  set adc_enc_n_o [ create_bd_port -dir O adc_enc_n_o ]
  set adc_enc_p_o [ create_bd_port -dir O adc_enc_p_o ]
  set dac_clk_o [ create_bd_port -dir O dac_clk_o ]
  set dac_dat_o [ create_bd_port -dir O -from 13 -to 0 dac_dat_o ]
  set dac_pwm_o [ create_bd_port -dir O -from 3 -to 0 dac_pwm_o ]
  set dac_rst_o [ create_bd_port -dir O dac_rst_o ]
  set dac_sel_o [ create_bd_port -dir O dac_sel_o ]
  set dac_wrt_o [ create_bd_port -dir O dac_wrt_o ]
  set daisy_n_i [ create_bd_port -dir I -from 1 -to 0 daisy_n_i ]
  set daisy_n_o [ create_bd_port -dir O -from 1 -to 0 daisy_n_o ]
  set daisy_p_i [ create_bd_port -dir I -from 1 -to 0 daisy_p_i ]
  set daisy_p_o [ create_bd_port -dir O -from 1 -to 0 daisy_p_o ]
  set exp_n_io [ create_bd_port -dir IO -from 7 -to 0 exp_n_io ]
  set exp_p_io [ create_bd_port -dir IO -from 7 -to 0 exp_p_io ]
  set led_o [ create_bd_port -dir O -from 7 -to 0 led_o ]
  set reset_rtl_0 [ create_bd_port -dir I -type rst reset_rtl_0 ]
  set_property -dict [ list \
   CONFIG.POLARITY {ACTIVE_HIGH} \
 ] $reset_rtl_0

  # Create instance: PS
  create_hier_cell_PS [current_bd_instance .] PS

  # Create instance: PS_Amplitude_Controller
  create_hier_cell_PS_Amplitude_Controller [current_bd_instance .] PS_Amplitude_Controller

  # Create instance: PS_Phase_Controller
  create_hier_cell_PS_Phase_Controller [current_bd_instance .] PS_Phase_Controller

  # Create instance: PS_data_transport
  create_hier_cell_PS_data_transport [current_bd_instance .] PS_data_transport

  # Create instance: VolumeAdjuster16_14_and_QCtrl, and set properties
  set block_name VolumeAdjuster16_14
  set block_cell_name VolumeAdjuster16_14_and_QCtrl
  if { [catch {set VolumeAdjuster16_14_and_QCtrl [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $VolumeAdjuster16_14_and_QCtrl eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: axis_dc_filter_0, and set properties
  set block_name axis_dc_filter
  set block_cell_name axis_dc_filter_0
  if { [catch {set axis_dc_filter_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $axis_dc_filter_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: axis_decimator_0, and set properties
  set block_name axis_decimator
  set block_cell_name axis_decimator_0
  if { [catch {set axis_decimator_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $axis_decimator_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: axis_red_pitaya_adc_0, and set properties
  set axis_red_pitaya_adc_0 [ create_bd_cell -type ip -vlnv pavel-demin:user:axis_red_pitaya_adc:1.0 axis_red_pitaya_adc_0 ]

  set_property -dict [ list \
   CONFIG.FREQ_HZ {125000000} \
 ] [get_bd_pins /axis_red_pitaya_adc_0/adc_clk_n]

  # Create instance: axis_red_pitaya_dac_0, and set properties
  set axis_red_pitaya_dac_0 [ create_bd_cell -type ip -vlnv pavel-demin:user:axis_red_pitaya_dac:1.0 axis_red_pitaya_dac_0 ]

  # Create instance: axis_sc26_to_14_0, and set properties
  set block_name axis_sc28_to_14
  set block_cell_name axis_sc26_to_14_0
  if { [catch {set axis_sc26_to_14_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $axis_sc26_to_14_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.SRC_BITS {26} \
 ] $axis_sc26_to_14_0

  # Create instance: cfg_select_amplitude_control, and set properties
  set block_name cfg_select
  set block_cell_name cfg_select_amplitude_control
  if { [catch {set cfg_select_amplitude_control [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_select_amplitude_control eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.CFG_SWBIT {1} \
   CONFIG.SRC_ADDR {3} \
 ] $cfg_select_amplitude_control

  # Create instance: cfg_select_lck_ampl_control, and set properties
  set block_name cfg_select
  set block_cell_name cfg_select_lck_ampl_control
  if { [catch {set cfg_select_lck_ampl_control [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_select_lck_ampl_control eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.CFG_SWBIT {4} \
   CONFIG.SRC_ADDR {3} \
 ] $cfg_select_lck_ampl_control

  # Create instance: cfg_select_lck_phase_control1, and set properties
  set block_name cfg_select
  set block_cell_name cfg_select_lck_phase_control1
  if { [catch {set cfg_select_lck_phase_control1 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_select_lck_phase_control1 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.CFG_SWBIT {5} \
   CONFIG.SRC_ADDR {3} \
 ] $cfg_select_lck_phase_control1

  # Create instance: cfg_select_phase_control, and set properties
  set block_name cfg_select
  set block_cell_name cfg_select_phase_control
  if { [catch {set cfg_select_phase_control [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_select_phase_control eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.SRC_ADDR {3} \
 ] $cfg_select_phase_control

  # Create instance: cfg_select_qcontrol, and set properties
  set block_name cfg_select
  set block_cell_name cfg_select_qcontrol
  if { [catch {set cfg_select_qcontrol [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_select_qcontrol eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.CFG_SWBIT {3} \
   CONFIG.SRC_ADDR {3} \
 ] $cfg_select_qcontrol

  # Create instance: cfg_to_axis_Atau, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_axis_Atau
  if { [catch {set cfg_to_axis_Atau [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_axis_Atau eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.SRC_ADDR {27} \
 ] $cfg_to_axis_Atau

  # Create instance: cfg_to_axis_dc, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_axis_dc
  if { [catch {set cfg_to_axis_dc [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_axis_dc eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {32} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {5} \
 ] $cfg_to_axis_dc

  # Create instance: cfg_to_axis_dc_tau, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_axis_dc_tau
  if { [catch {set cfg_to_axis_dc_tau [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_axis_dc_tau eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.SRC_ADDR {28} \
 ] $cfg_to_axis_dc_tau

  # Create instance: cfg_to_axis_qc_delay, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_axis_qc_delay
  if { [catch {set cfg_to_axis_qc_delay [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_axis_qc_delay eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {16} \
   CONFIG.SRC_ADDR {29} \
   CONFIG.SRC_BITS {16} \
 ] $cfg_to_axis_qc_delay

  # Create instance: cfg_to_axis_qc_gain, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_axis_qc_gain
  if { [catch {set cfg_to_axis_qc_gain [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_axis_qc_gain eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {16} \
   CONFIG.SRC_ADDR {29} \
   CONFIG.SRC_BITS {32} \
 ] $cfg_to_axis_qc_gain

  # Create instance: cfg_to_axis_tau, and set properties
  set block_name cfg_to_axis
  set block_cell_name cfg_to_axis_tau
  if { [catch {set cfg_to_axis_tau [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $cfg_to_axis_tau eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.DST_WIDTH {32} \
   CONFIG.MAXIS_TDATA_WIDTH {32} \
   CONFIG.SRC_ADDR {4} \
 ] $cfg_to_axis_tau

  # Create instance: clk_wiz_0, and set properties
  set clk_wiz_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_0 ]
  set_property -dict [ list \
   CONFIG.CLKIN1_JITTER_PS {80.0} \
   CONFIG.CLKIN2_JITTER_PS {166.66} \
   CONFIG.CLKOUT1_JITTER {104.759} \
   CONFIG.CLKOUT1_PHASE_ERROR {96.948} \
   CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {250} \
   CONFIG.CLKOUT2_JITTER {137.150} \
   CONFIG.CLKOUT2_PHASE_ERROR {96.948} \
   CONFIG.CLKOUT2_REQUESTED_OUT_FREQ {62.5} \
   CONFIG.CLKOUT2_USED {true} \
   CONFIG.MMCM_CLKFBOUT_MULT_F {8.000} \
   CONFIG.MMCM_CLKIN1_PERIOD {8.000} \
   CONFIG.MMCM_CLKIN2_PERIOD {10.000} \
   CONFIG.MMCM_CLKOUT0_DIVIDE_F {4.000} \
   CONFIG.MMCM_CLKOUT1_DIVIDE {16} \
   CONFIG.MMCM_DIVCLK_DIVIDE {1} \
   CONFIG.NUM_OUT_CLKS {2} \
   CONFIG.PRIM_IN_FREQ {125} \
   CONFIG.SECONDARY_SOURCE {Single_ended_clock_capable_pin} \
   CONFIG.USE_INCLK_SWITCHOVER {false} \
   CONFIG.USE_RESET {false} \
 ] $clk_wiz_0

  # Create instance: cordic_atan_xy, and set properties
  set cordic_atan_xy [ create_bd_cell -type ip -vlnv xilinx.com:ip:cordic:6.0 cordic_atan_xy ]
  set_property -dict [ list \
   CONFIG.Coarse_Rotation {true} \
   CONFIG.Data_Format {SignedFraction} \
   CONFIG.Functional_Selection {Arc_Tan} \
   CONFIG.Input_Width {28} \
   CONFIG.Output_Width {24} \
   CONFIG.Pipelining_Mode {Optimal} \
 ] $cordic_atan_xy

  # Create instance: cordic_sqrt, and set properties
  set cordic_sqrt [ create_bd_cell -type ip -vlnv xilinx.com:ip:cordic:6.0 cordic_sqrt ]
  set_property -dict [ list \
   CONFIG.Coarse_Rotation {false} \
   CONFIG.Data_Format {UnsignedFraction} \
   CONFIG.Functional_Selection {Square_Root} \
   CONFIG.Input_Width {45} \
   CONFIG.Output_Width {24} \
   CONFIG.Pipelining_Mode {Optimal} \
 ] $cordic_sqrt

  # Create instance: dds_compiler_0, and set properties
  set dds_compiler_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:dds_compiler:6.0 dds_compiler_0 ]
  set_property -dict [ list \
   CONFIG.DATA_Has_TLAST {Not_Required} \
   CONFIG.DDS_Clock_Rate {62.5} \
   CONFIG.Frequency_Resolution {0.00001} \
   CONFIG.Has_Phase_Out {true} \
   CONFIG.Latency {10} \
   CONFIG.M_DATA_Has_TUSER {Not_Required} \
   CONFIG.Noise_Shaping {Auto} \
   CONFIG.Output_Frequency1 {0} \
   CONFIG.Output_Width {26} \
   CONFIG.PINC1 {0} \
   CONFIG.Phase_Increment {Streaming} \
   CONFIG.Phase_Width {43} \
   CONFIG.Phase_offset {None} \
   CONFIG.S_PHASE_Has_TUSER {Not_Required} \
   CONFIG.Spurious_Free_Dynamic_Range {150} \
 ] $dds_compiler_0

  # Create instance: led_connect_0, and set properties
  set block_name led_connect
  set block_cell_name led_connect_0
  if { [catch {set led_connect_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $led_connect_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: lms_phase_amplitude_detector_0, and set properties
  set block_name lms_phase_amplitude_detector
  set block_cell_name lms_phase_amplitude_detector_0
  if { [catch {set lms_phase_amplitude_detector_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $lms_phase_amplitude_detector_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [ list \
   CONFIG.SC_DATA_WIDTH {26} \
 ] $lms_phase_amplitude_detector_0

  # Create instance: util_ds_buf_1, and set properties
  set util_ds_buf_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:util_ds_buf:2.1 util_ds_buf_1 ]
  set_property -dict [ list \
   CONFIG.C_SIZE {2} \
 ] $util_ds_buf_1

  # Create instance: util_ds_buf_2, and set properties
  set util_ds_buf_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:util_ds_buf:2.1 util_ds_buf_2 ]
  set_property -dict [ list \
   CONFIG.C_BUF_TYPE {OBUFDS} \
   CONFIG.C_SIZE {2} \
 ] $util_ds_buf_2

  # Create interface connections
  connect_bd_intf_net -intf_net PS_Amplitude_Controller_M_AXIS_CONTROL [get_bd_intf_pins PS_Amplitude_Controller/M_AXIS_CONTROL] [get_bd_intf_pins VolumeAdjuster16_14_and_QCtrl/SV_AXIS]
  connect_bd_intf_net -intf_net PS_Amplitude_Controller_M_AXIS_CONTROL2 [get_bd_intf_pins PS_Amplitude_Controller/M_AXIS_CONTROL2] [get_bd_intf_pins PS_data_transport/S_AXIS2]
  connect_bd_intf_net -intf_net PS_Amplitude_Controller_M_AXIS_PASS2 [get_bd_intf_pins PS_Amplitude_Controller/M_AXIS_PASS2] [get_bd_intf_pins PS_data_transport/S_AXIS5]
  connect_bd_intf_net -intf_net PS_BRAM_PORTA [get_bd_intf_pins PS/BRAM_PORTA] [get_bd_intf_pins PS_data_transport/BRAM_PORTB]
  connect_bd_intf_net -intf_net PS_Phase_Controller_M_AXIS_CONTROL [get_bd_intf_pins PS_Phase_Controller/M_AXIS_CONTROL] [get_bd_intf_pins dds_compiler_0/S_AXIS_PHASE]
  connect_bd_intf_net -intf_net PS_Phase_Controller_M_AXIS_CONTROL2 [get_bd_intf_pins PS_Phase_Controller/M_AXIS_CONTROL2] [get_bd_intf_pins PS_data_transport/S_AXIS3]
  connect_bd_intf_net -intf_net PS_Phase_Controller_M_AXIS_PASS2 [get_bd_intf_pins PS_Phase_Controller/M_AXIS_PASS2] [get_bd_intf_pins PS_data_transport/S_AXIS4]
  connect_bd_intf_net -intf_net PS_data_transport_M_AXIS_aux [get_bd_intf_pins PS_data_transport/M_AXIS_aux] [get_bd_intf_pins axis_sc26_to_14_0/S_AXIS_aux]
  connect_bd_intf_net -intf_net S_AXIS1_1 [get_bd_intf_pins PS_data_transport/S_AXIS1] [get_bd_intf_pins axis_decimator_0/M_AXIS_S01]
  connect_bd_intf_net -intf_net VolumeAdjuster16_14_0_M_AXIS [get_bd_intf_pins VolumeAdjuster16_14_and_QCtrl/M_AXIS] [get_bd_intf_pins axis_red_pitaya_dac_0/S_AXIS]
  connect_bd_intf_net -intf_net axis_dc_filter_0_M_AXIS [get_bd_intf_pins axis_dc_filter_0/M_AXIS_AC_LMS] [get_bd_intf_pins lms_phase_amplitude_detector_0/S_AXIS_SIGNAL]
  connect_bd_intf_net -intf_net axis_dc_filter_0_M_AXIS_ACDC [get_bd_intf_pins PS_data_transport/S_AXIS6] [get_bd_intf_pins axis_dc_filter_0/M_AXIS_ACDC]
  connect_bd_intf_net -intf_net axis_decimator_0_M_AXIS_S0 [get_bd_intf_pins axis_dc_filter_0/S_AXIS] [get_bd_intf_pins axis_decimator_0/M_AXIS_S0]
  connect_bd_intf_net -intf_net axis_red_pitaya_adc_0_M_AXIS [get_bd_intf_pins axis_decimator_0/S_AXIS_SIGNAL] [get_bd_intf_pins axis_red_pitaya_adc_0/M_AXIS]
  connect_bd_intf_net -intf_net axis_sc26_to_14_0_M_AXIS [get_bd_intf_pins VolumeAdjuster16_14_and_QCtrl/S_AXIS] [get_bd_intf_pins axis_sc26_to_14_0/M_AXIS]
  connect_bd_intf_net -intf_net cordic_atan_xy_M_AXIS_DOUT [get_bd_intf_pins PS_Phase_Controller/S_AXIS] [get_bd_intf_pins cordic_atan_xy/M_AXIS_DOUT]
  connect_bd_intf_net -intf_net cordic_sqrt_M_AXIS_DOUT [get_bd_intf_pins PS_Amplitude_Controller/S_AXIS] [get_bd_intf_pins cordic_sqrt/M_AXIS_DOUT]
  connect_bd_intf_net -intf_net dds_compiler_0_M_AXIS_DATA [get_bd_intf_pins dds_compiler_0/M_AXIS_DATA] [get_bd_intf_pins lms_phase_amplitude_detector_0/S_AXIS_SC]
  connect_bd_intf_net -intf_net lms_phase_amplitude_detector_0_M_AXIS_AM2 [get_bd_intf_pins cordic_sqrt/S_AXIS_CARTESIAN] [get_bd_intf_pins lms_phase_amplitude_detector_0/M_AXIS_AM2]
  connect_bd_intf_net -intf_net lms_phase_amplitude_detector_0_M_AXIS_LockInX [get_bd_intf_pins PS_data_transport/S_AXIS7] [get_bd_intf_pins lms_phase_amplitude_detector_0/M_AXIS_LockInX]
  connect_bd_intf_net -intf_net lms_phase_amplitude_detector_0_M_AXIS_LockInY [get_bd_intf_pins PS_data_transport/S_AXIS8] [get_bd_intf_pins lms_phase_amplitude_detector_0/M_AXIS_LockInY]
  connect_bd_intf_net -intf_net lms_phase_amplitude_detector_0_M_AXIS_SC [get_bd_intf_pins axis_sc26_to_14_0/S_AXIS] [get_bd_intf_pins lms_phase_amplitude_detector_0/M_AXIS_SC]
  connect_bd_intf_net -intf_net lms_phase_amplitude_detector_0_M_AXIS_SIGNAL_M [get_bd_intf_pins VolumeAdjuster16_14_and_QCtrl/S_AXIS_SIGNAL_M] [get_bd_intf_pins axis_dc_filter_0/M_AXIS_AC16]
  connect_bd_intf_net -intf_net lms_phase_amplitude_detector_0_M_AXIS_XY [get_bd_intf_pins cordic_atan_xy/S_AXIS_CARTESIAN] [get_bd_intf_pins lms_phase_amplitude_detector_0/M_AXIS_XY]
  connect_bd_intf_net -intf_net processing_system7_0_DDR [get_bd_intf_ports DDR] [get_bd_intf_pins PS/DDR]
  connect_bd_intf_net -intf_net processing_system7_0_FIXED_IO [get_bd_intf_ports FIXED_IO] [get_bd_intf_pins PS/FIXED_IO]

  # Create port connections
  connect_bd_net -net Net [get_bd_ports exp_p_io] [get_bd_pins PS_data_transport/exp_p_io]
  connect_bd_net -net Net1 [get_bd_ports exp_n_io] [get_bd_pins PS_data_transport/exp_n_io]
  connect_bd_net -net PS_Amplitude_Controller_status_max [get_bd_pins PS_Amplitude_Controller/status_max] [get_bd_pins led_connect_0/led3]
  connect_bd_net -net PS_Amplitude_Controller_status_min [get_bd_pins PS_Amplitude_Controller/status_min] [get_bd_pins led_connect_0/led2]
  connect_bd_net -net PS_Phase_Controller_mon_control_B [get_bd_pins PS_Phase_Controller/mon_control_B] [get_bd_pins lms_phase_amplitude_detector_0/DDS_dphi]
  connect_bd_net -net PS_Phase_Controller_status_max [get_bd_pins PS_Phase_Controller/status_max] [get_bd_pins led_connect_0/led6]
  connect_bd_net -net PS_Phase_Controller_status_min [get_bd_pins PS_Phase_Controller/status_min] [get_bd_pins led_connect_0/led5]
  connect_bd_net -net PS_cfg_data [get_bd_pins PS/cfg_data] [get_bd_pins PS_Amplitude_Controller/cfg] [get_bd_pins PS_Phase_Controller/cfg] [get_bd_pins PS_data_transport/cfg] [get_bd_pins cfg_select_amplitude_control/cfg] [get_bd_pins cfg_select_lck_ampl_control/cfg] [get_bd_pins cfg_select_lck_phase_control1/cfg] [get_bd_pins cfg_select_phase_control/cfg] [get_bd_pins cfg_select_qcontrol/cfg] [get_bd_pins cfg_to_axis_Atau/cfg] [get_bd_pins cfg_to_axis_dc/cfg] [get_bd_pins cfg_to_axis_dc_tau/cfg] [get_bd_pins cfg_to_axis_qc_delay/cfg] [get_bd_pins cfg_to_axis_qc_gain/cfg] [get_bd_pins cfg_to_axis_tau/cfg]
  connect_bd_net -net PS_data_transport_dbgA [get_bd_pins PS/gpio_io_5_x12] [get_bd_pins PS_data_transport/dbgA]
  connect_bd_net -net PS_data_transport_dbgB [get_bd_pins PS/gpio_io_5_x11] [get_bd_pins PS_data_transport/dbgB]
  connect_bd_net -net PS_data_transport_finished_state [get_bd_pins PS_data_transport/finished_state] [get_bd_pins led_connect_0/led8]
  connect_bd_net -net PS_data_transport_init_state [get_bd_pins PS_data_transport/init_state] [get_bd_pins led_connect_0/led7]
  connect_bd_net -net adc_clk_n_i_1 [get_bd_ports adc_clk_n_i] [get_bd_pins axis_red_pitaya_adc_0/adc_clk_n]
  connect_bd_net -net adc_clk_p_i_1 [get_bd_ports adc_clk_p_i] [get_bd_pins axis_red_pitaya_adc_0/adc_clk_p]
  connect_bd_net -net adc_dat_a_i_1 [get_bd_ports adc_dat_a_i] [get_bd_pins axis_red_pitaya_adc_0/adc_dat_a]
  connect_bd_net -net adc_dat_b_i_1 [get_bd_ports adc_dat_b_i] [get_bd_pins axis_red_pitaya_adc_0/adc_dat_b]
  connect_bd_net -net amplitude_controller_mon_signal [get_bd_pins PS/gpio_io_1_x4] [get_bd_pins PS_Amplitude_Controller/mon_signal]
  connect_bd_net -net axis_dc_filter_0_dbg_m [get_bd_pins PS/gpio_io_1_x3] [get_bd_pins axis_dc_filter_0/dbg_m]
  connect_bd_net -net axis_dc_filter_0_dbg_mdc [get_bd_pins PS/gpio_io_2_x5] [get_bd_pins axis_dc_filter_0/dbg_mdc]
  connect_bd_net -net axis_red_pitaya_adc_0_adc_clk [get_bd_pins PS/slowest_sync_clk] [get_bd_pins PS_Amplitude_Controller/aclk] [get_bd_pins PS_Phase_Controller/aclk] [get_bd_pins PS_data_transport/a_clk] [get_bd_pins VolumeAdjuster16_14_and_QCtrl/a_clk] [get_bd_pins axis_dc_filter_0/aclk] [get_bd_pins axis_decimator_0/dec_aclk] [get_bd_pins axis_sc26_to_14_0/a_clk] [get_bd_pins cfg_to_axis_Atau/a_clk] [get_bd_pins cfg_to_axis_dc/a_clk] [get_bd_pins cfg_to_axis_dc_tau/a_clk] [get_bd_pins cfg_to_axis_qc_delay/a_clk] [get_bd_pins cfg_to_axis_qc_gain/a_clk] [get_bd_pins cfg_to_axis_tau/a_clk] [get_bd_pins clk_wiz_0/clk_out2] [get_bd_pins cordic_atan_xy/aclk] [get_bd_pins cordic_sqrt/aclk] [get_bd_pins dds_compiler_0/aclk] [get_bd_pins lms_phase_amplitude_detector_0/aclk]
  connect_bd_net -net axis_red_pitaya_adc_0_adc_clk1 [get_bd_pins PS_data_transport/a2_clk] [get_bd_pins VolumeAdjuster16_14_and_QCtrl/adc_clk] [get_bd_pins axis_decimator_0/adc_clk] [get_bd_pins axis_red_pitaya_adc_0/adc_clk] [get_bd_pins axis_red_pitaya_dac_0/aclk] [get_bd_pins clk_wiz_0/clk_in1]
  connect_bd_net -net axis_red_pitaya_adc_0_adc_csn [get_bd_ports adc_csn_o] [get_bd_pins axis_red_pitaya_adc_0/adc_csn]
  connect_bd_net -net axis_red_pitaya_dac_0_dac_clk [get_bd_ports dac_clk_o] [get_bd_pins axis_red_pitaya_dac_0/dac_clk]
  connect_bd_net -net axis_red_pitaya_dac_0_dac_dat [get_bd_ports dac_dat_o] [get_bd_pins axis_red_pitaya_dac_0/dac_dat]
  connect_bd_net -net axis_red_pitaya_dac_0_dac_rst [get_bd_ports dac_rst_o] [get_bd_pins axis_red_pitaya_dac_0/dac_rst]
  connect_bd_net -net axis_red_pitaya_dac_0_dac_sel [get_bd_ports dac_sel_o] [get_bd_pins axis_red_pitaya_dac_0/dac_sel]
  connect_bd_net -net axis_red_pitaya_dac_0_dac_wrt [get_bd_ports dac_wrt_o] [get_bd_pins axis_red_pitaya_dac_0/dac_wrt]
  connect_bd_net -net cfg_axis_phaseinc_status [get_bd_pins PS_Phase_Controller/enable] [get_bd_pins cfg_select_phase_control/status] [get_bd_pins led_connect_0/led4]
  connect_bd_net -net cfg_axis_volume_status [get_bd_pins PS_Amplitude_Controller/enable] [get_bd_pins cfg_select_amplitude_control/status] [get_bd_pins led_connect_0/led1]
  connect_bd_net -net cfg_select_lck_ampl_control_status [get_bd_pins cfg_select_lck_ampl_control/status] [get_bd_pins lms_phase_amplitude_detector_0/lck_ampl]
  connect_bd_net -net cfg_select_lck_phase_control1_status [get_bd_pins cfg_select_lck_phase_control1/status] [get_bd_pins lms_phase_amplitude_detector_0/lck_phase]
  connect_bd_net -net cfg_select_qcontrol_status [get_bd_pins VolumeAdjuster16_14_and_QCtrl/QC_enable] [get_bd_pins cfg_select_qcontrol/status]
  connect_bd_net -net cfg_to_axis_Atau_M_AXIS_tdata [get_bd_pins cfg_to_axis_Atau/M_AXIS_tdata] [get_bd_pins lms_phase_amplitude_detector_0/Atau]
  connect_bd_net -net cfg_to_axis_dc_M_AXIS_tdata [get_bd_pins axis_dc_filter_0/dc] [get_bd_pins cfg_to_axis_dc/M_AXIS_tdata]
  connect_bd_net -net cfg_to_axis_dc_tau_M_AXIS_tdata [get_bd_pins axis_dc_filter_0/dc_tau] [get_bd_pins cfg_to_axis_dc_tau/M_AXIS_tdata]
  connect_bd_net -net cfg_to_axis_qc_delay_data [get_bd_pins VolumeAdjuster16_14_and_QCtrl/QC_delay] [get_bd_pins cfg_to_axis_qc_delay/data]
  connect_bd_net -net cfg_to_axis_qc_gain_data [get_bd_pins VolumeAdjuster16_14_and_QCtrl/QC_gain] [get_bd_pins cfg_to_axis_qc_gain/data]
  connect_bd_net -net cfg_to_axis_tau_M_AXIS_tdata [get_bd_pins cfg_to_axis_tau/M_AXIS_tdata] [get_bd_pins lms_phase_amplitude_detector_0/tau]
  connect_bd_net -net clk_wiz_0_clk_out1 [get_bd_pins axis_red_pitaya_dac_0/ddr_clk] [get_bd_pins clk_wiz_0/clk_out1]
  connect_bd_net -net clk_wiz_0_locked [get_bd_pins axis_red_pitaya_dac_0/locked] [get_bd_pins clk_wiz_0/locked]
  connect_bd_net -net daisy_n_i_1 [get_bd_ports daisy_n_i] [get_bd_pins util_ds_buf_1/IBUF_DS_N]
  connect_bd_net -net daisy_p_i_1 [get_bd_ports daisy_p_i] [get_bd_pins util_ds_buf_1/IBUF_DS_P]
  connect_bd_net -net gpio2_io_i_1 [get_bd_pins PS/gpio_io_4_x10] [get_bd_pins PS_Phase_Controller/mon_signal]
  connect_bd_net -net gpio_io_2_x6_1 [get_bd_pins PS/gpio_io_2_x6] [get_bd_pins PS_Amplitude_Controller/mon_control_lower32]
  connect_bd_net -net gpio_io_3_x7_1 [get_bd_pins PS/gpio_io_3_x7] [get_bd_pins PS_Amplitude_Controller/mon_control]
  connect_bd_net -net gpio_io_3_x8_1 [get_bd_pins PS/gpio_io_3_x8] [get_bd_pins PS_Phase_Controller/mon_control]
  connect_bd_net -net gpio_io_4_x9_1 [get_bd_pins PS/gpio_io_4_x9] [get_bd_pins PS_Phase_Controller/mon_control_lower32]
  connect_bd_net -net led_connect_0_leds [get_bd_ports led_o] [get_bd_pins led_connect_0/leds]
  connect_bd_net -net lms_phase_amplitude_detector_0_Aout [get_bd_pins PS/gpio_io_0_x1] [get_bd_pins lms_phase_amplitude_detector_0/M_AXIS_Aout_tdata]
  connect_bd_net -net lms_phase_amplitude_detector_0_Bout [get_bd_pins PS/gpio_io_0_x2] [get_bd_pins lms_phase_amplitude_detector_0/M_AXIS_Bout_tdata]
  connect_bd_net -net lms_phase_amplitude_detector_0_sc_zero_x [get_bd_pins axis_dc_filter_0/sc_zero] [get_bd_pins lms_phase_amplitude_detector_0/sc_zero_x]
  connect_bd_net -net util_ds_buf_1_IBUF_OUT [get_bd_pins util_ds_buf_1/IBUF_OUT] [get_bd_pins util_ds_buf_2/OBUF_IN]
  connect_bd_net -net util_ds_buf_2_OBUF_DS_N [get_bd_ports daisy_n_o] [get_bd_pins util_ds_buf_2/OBUF_DS_N]
  connect_bd_net -net util_ds_buf_2_OBUF_DS_P [get_bd_ports daisy_p_o] [get_bd_pins util_ds_buf_2/OBUF_DS_P]

  # Create address segments
  assign_bd_address -offset 0x40000000 -range 0x00200000 -target_address_space [get_bd_addr_spaces PS/processing_system7_0/Data] [get_bd_addr_segs PS/axi_bram_reader_0/s00_axi/reg0] -force
  assign_bd_address -offset 0x42000000 -range 0x00001000 -target_address_space [get_bd_addr_spaces PS/processing_system7_0/Data] [get_bd_addr_segs PS/axi_cfg_register_0/s_axi/reg0] -force
  assign_bd_address -offset 0x42001000 -range 0x00001000 -target_address_space [get_bd_addr_spaces PS/processing_system7_0/Data] [get_bd_addr_segs PS/axi_gpio_0/S_AXI/Reg] -force
  assign_bd_address -offset 0x42002000 -range 0x00001000 -target_address_space [get_bd_addr_spaces PS/processing_system7_0/Data] [get_bd_addr_segs PS/axi_gpio_1/S_AXI/Reg] -force
  assign_bd_address -offset 0x42003000 -range 0x00001000 -target_address_space [get_bd_addr_spaces PS/processing_system7_0/Data] [get_bd_addr_segs PS/axi_gpio_2/S_AXI/Reg] -force
  assign_bd_address -offset 0x42004000 -range 0x00001000 -target_address_space [get_bd_addr_spaces PS/processing_system7_0/Data] [get_bd_addr_segs PS/axi_gpio_3/S_AXI/Reg] -force
  assign_bd_address -offset 0x42005000 -range 0x00001000 -target_address_space [get_bd_addr_spaces PS/processing_system7_0/Data] [get_bd_addr_segs PS/axi_gpio_4/S_AXI/Reg] -force
  assign_bd_address -offset 0x42006000 -range 0x00001000 -target_address_space [get_bd_addr_spaces PS/processing_system7_0/Data] [get_bd_addr_segs PS/axi_gpio_5/S_AXI/Reg] -force


  # Restore current instance
  current_bd_instance $oldCurInst

  validate_bd_design
  save_bd_design
}
# End of create_root_design()


##################################################################
# MAIN FLOW
##################################################################

create_root_design ""


