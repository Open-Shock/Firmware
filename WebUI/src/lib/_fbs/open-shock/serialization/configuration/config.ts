// automatically generated by the FlatBuffers compiler, do not modify

import * as flatbuffers from 'flatbuffers';

import { BackendConfig } from '../../../open-shock/serialization/configuration/backend-config.js';
import { CaptivePortalConfig } from '../../../open-shock/serialization/configuration/captive-portal-config.js';
import { RFConfig } from '../../../open-shock/serialization/configuration/rfconfig.js';
import { WiFiConfig } from '../../../open-shock/serialization/configuration/wi-fi-config.js';


export class Config {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):Config {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsConfig(bb:flatbuffers.ByteBuffer, obj?:Config):Config {
  return (obj || new Config()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsConfig(bb:flatbuffers.ByteBuffer, obj?:Config):Config {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new Config()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

/**
 * RF Transmitter configuration
 */
rf(obj?:RFConfig):RFConfig|null {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? (obj || new RFConfig()).__init(this.bb!.__indirect(this.bb_pos + offset), this.bb!) : null;
}

/**
 * WiFi configuration
 */
wifi(obj?:WiFiConfig):WiFiConfig|null {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? (obj || new WiFiConfig()).__init(this.bb!.__indirect(this.bb_pos + offset), this.bb!) : null;
}

/**
 * Captive portal configuration
 */
captivePortal(obj?:CaptivePortalConfig):CaptivePortalConfig|null {
  const offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? (obj || new CaptivePortalConfig()).__init(this.bb!.__indirect(this.bb_pos + offset), this.bb!) : null;
}

/**
 * Backend configuration
 */
backend(obj?:BackendConfig):BackendConfig|null {
  const offset = this.bb!.__offset(this.bb_pos, 10);
  return offset ? (obj || new BackendConfig()).__init(this.bb!.__indirect(this.bb_pos + offset), this.bb!) : null;
}

static startConfig(builder:flatbuffers.Builder) {
  builder.startObject(4);
}

static addRf(builder:flatbuffers.Builder, rfOffset:flatbuffers.Offset) {
  builder.addFieldOffset(0, rfOffset, 0);
}

static addWifi(builder:flatbuffers.Builder, wifiOffset:flatbuffers.Offset) {
  builder.addFieldOffset(1, wifiOffset, 0);
}

static addCaptivePortal(builder:flatbuffers.Builder, captivePortalOffset:flatbuffers.Offset) {
  builder.addFieldOffset(2, captivePortalOffset, 0);
}

static addBackend(builder:flatbuffers.Builder, backendOffset:flatbuffers.Offset) {
  builder.addFieldOffset(3, backendOffset, 0);
}

static endConfig(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

}
