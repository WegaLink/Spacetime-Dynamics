<?xml version="1.0" encoding="ASCII"?>
<ResourceModel:App xmi:version="2.0" xmlns:xmi="http://www.omg.org/XMI" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:ResourceModel="http://www.infineon.com/Davex/Resource.ecore" name="I2C_CONFIG" URI="http://resources/4.0.6/app/I2C_CONFIG/0" description="Configures a USIC channel as a master/slave to perform transmit &amp; receive operations using the I2C protocol" mode="NOTSHARABLE" version="4.0.6" minDaveVersion="4.1.2" instanceLabel="I2C_CONFIG_BME280" appLabel="">
  <properties provideInit="true"/>
  <virtualSignals name="sda_in" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_datain0" hwSignal="dx0ins" hwResource="//@hwResources.0" visible="true">
    <upwardMapList xsi:type="ResourceModel:Connections" href="../../DIGITAL_IO/v4_0_16/DIGITAL_IO_3.app#//@connections.2"/>
  </virtualSignals>
  <virtualSignals name="scl_in" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_sclkin" hwSignal="dx1ins" hwResource="//@hwResources.0" visible="true">
    <upwardMapList xsi:type="ResourceModel:Connections" href="../../DIGITAL_IO/v4_0_16/DIGITAL_IO_4.app#//@connections.2"/>
  </virtualSignals>
  <virtualSignals name="transmit_data_validation" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_tx_validation" hwSignal="dx2ins" hwResource="//@hwResources.0" required="false"/>
  <virtualSignals name="sda_out" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_dout0" hwSignal="dout0" hwResource="//@hwResources.0" visible="true"/>
  <virtualSignals name="scl_out" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_sclkout" hwSignal="sclkout" hwResource="//@hwResources.0" visible="true"/>
  <virtualSignals name="dx1ins_out" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_dx1ins_sys_out" hwSignal="dx1ins" hwResource="//@hwResources.0" visible="true"/>
  <virtualSignals name="dx2ins_out" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_dx2ins_sys_out" hwSignal="dx2ins" hwResource="//@hwResources.0" required="false"/>
  <virtualSignals name="transmit_buffer_event" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_standard_tx_event" hwSignal="transmit_buffer_int" hwResource="//@hwResources.0" visible="true"/>
  <virtualSignals name="standard_receive_event" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_standard_rx_event" hwSignal="standard_receive_int" hwResource="//@hwResources.0" visible="true"/>
  <virtualSignals name="transmit_shift_event" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_tx_shift_event" hwSignal="transmit_shift_int" hwResource="//@hwResources.0" visible="true"/>
  <virtualSignals name="receive_start_event" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_rx_start_event" hwSignal="receive_start_int" hwResource="//@hwResources.0" visible="true"/>
  <virtualSignals name="alternate_receive_event" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_alt_rx_event" hwSignal="alternate_receive_int" hwResource="//@hwResources.0" visible="true"/>
  <virtualSignals name="protocol_specific_event" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_proto_specfic_event" hwSignal="protocol_specific_int" hwResource="//@hwResources.0" visible="true"/>
  <virtualSignals name="fifo_standard_transmit_buffer_event" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_fifo_tx_event" hwSignal="standard_transmit_buffer_int" hwResource="//@hwResources.0" required="false"/>
  <virtualSignals name="fifo_transmit_buffer_error_event" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_fifo_tx_err_event" hwSignal="transmit_buffer_error_int" hwResource="//@hwResources.0" required="false"/>
  <virtualSignals name="fifo_standard_receive_buffer_event" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_fifo_rx_event" hwSignal="standard_receive_buffer_int" hwResource="//@hwResources.0" required="false"/>
  <virtualSignals name="fifo_receive_buffer_error_event" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_fifo_rx_err_event" hwSignal="receive_buffer_error_int" hwResource="//@hwResources.0" required="false"/>
  <virtualSignals name="fifo_alternate_receive_buffer_event" URI="http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_fifo_alt_rx_event" hwSignal="alternate_receive_buffer_int" hwResource="//@hwResources.0" required="false"/>
  <requiredApps URI="http://resources/4.0.6/app/I2C_CONFIG/0/appres_clock" requiredAppName="CLOCK_XMC4" requiringMode="SHARABLE">
    <downwardMapList xsi:type="ResourceModel:App" href="../../CLOCK_XMC4/v4_0_22/CLOCK_XMC4_0.app#/"/>
  </requiredApps>
  <hwResources name="Channel" URI="http://resources/4.0.6/app/I2C_CONFIG/0/hwres_usic_channel" resourceGroupUri="peripheral/usic/*/channel/*" mResGrpUri="peripheral/usic/*/channel/*">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/usic1/usic1_1.dd#//@provided.43"/>
  </hwResources>
  <connections URI="http://resources/4.0.6/app/I2C_CONFIG/0/http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_dout0/http://resources/4.0.16/app/DIGITAL_IO/3/vs_digital_io_pad_pin" sourceSignal="sda_out" targetSignal="pin" srcVirtualSignal="//@virtualSignals.3">
    <downwardMapList xsi:type="ResourceModel:VirtualSignal" href="../../DIGITAL_IO/v4_0_16/DIGITAL_IO_3.app#//@virtualSignals.1"/>
    <targetVirtualSignal href="../../DIGITAL_IO/v4_0_16/DIGITAL_IO_3.app#//@virtualSignals.1"/>
  </connections>
  <connections URI="http://resources/4.0.6/app/I2C_CONFIG/0/http://resources/4.0.6/app/I2C_CONFIG/0/vs_i2c_sclkout/http://resources/4.0.16/app/DIGITAL_IO/4/vs_digital_io_pad_pin" sourceSignal="scl_out" targetSignal="pin" srcVirtualSignal="//@virtualSignals.4">
    <downwardMapList xsi:type="ResourceModel:VirtualSignal" href="../../DIGITAL_IO/v4_0_16/DIGITAL_IO_4.app#//@virtualSignals.1"/>
    <targetVirtualSignal href="../../DIGITAL_IO/v4_0_16/DIGITAL_IO_4.app#//@virtualSignals.1"/>
  </connections>
</ResourceModel:App>
