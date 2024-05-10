--[[ MyGame.Example.TypeAliases

  Automatically generated by the FlatBuffers compiler, do not modify.
  Or modify. I'm a message, not a cop.

  flatc version: 24.3.7

  Declared by  : //monster_test.fbs
  Rooting type : MyGame.Example.Monster (//monster_test.fbs)

--]]

local flatbuffers = require('flatbuffers')

local TypeAliases = {}
local mt = {}

function TypeAliases.New()
  local o = {}
  setmetatable(o, {__index = mt})
  return o
end

function mt:Init(buf, pos)
  self.view = flatbuffers.view.New(buf, pos)
end

function mt:I8()
  local o = self.view:Offset(4)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Int8, self.view.pos + o)
  end
  return 0
end

function mt:U8()
  local o = self.view:Offset(6)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint8, self.view.pos + o)
  end
  return 0
end

function mt:I16()
  local o = self.view:Offset(8)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Int16, self.view.pos + o)
  end
  return 0
end

function mt:U16()
  local o = self.view:Offset(10)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint16, self.view.pos + o)
  end
  return 0
end

function mt:I32()
  local o = self.view:Offset(12)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Int32, self.view.pos + o)
  end
  return 0
end

function mt:U32()
  local o = self.view:Offset(14)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint32, self.view.pos + o)
  end
  return 0
end

function mt:I64()
  local o = self.view:Offset(16)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Int64, self.view.pos + o)
  end
  return 0
end

function mt:U64()
  local o = self.view:Offset(18)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Uint64, self.view.pos + o)
  end
  return 0
end

function mt:F32()
  local o = self.view:Offset(20)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float32, self.view.pos + o)
  end
  return 0.0
end

function mt:F64()
  local o = self.view:Offset(22)
  if o ~= 0 then
    return self.view:Get(flatbuffers.N.Float64, self.view.pos + o)
  end
  return 0.0
end

function mt:V8(j)
  local o = self.view:Offset(24)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Int8, a + ((j-1) * 1))
  end
  return 0
end

function mt:V8AsString(start, stop)
  return self.view:VectorAsString(24, start, stop)
end

function mt:V8Length()
  local o = self.view:Offset(24)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function mt:Vf64(j)
  local o = self.view:Offset(26)
  if o ~= 0 then
    local a = self.view:Vector(o)
    return self.view:Get(flatbuffers.N.Float64, a + ((j-1) * 8))
  end
  return 0
end

function mt:Vf64Length()
  local o = self.view:Offset(26)
  if o ~= 0 then
    return self.view:VectorLen(o)
  end
  return 0
end

function TypeAliases.Start(builder)
  builder:StartObject(12)
end

function TypeAliases.AddI8(builder, i8)
  builder:PrependInt8Slot(0, i8, 0)
end

function TypeAliases.AddU8(builder, u8)
  builder:PrependUint8Slot(1, u8, 0)
end

function TypeAliases.AddI16(builder, i16)
  builder:PrependInt16Slot(2, i16, 0)
end

function TypeAliases.AddU16(builder, u16)
  builder:PrependUint16Slot(3, u16, 0)
end

function TypeAliases.AddI32(builder, i32)
  builder:PrependInt32Slot(4, i32, 0)
end

function TypeAliases.AddU32(builder, u32)
  builder:PrependUint32Slot(5, u32, 0)
end

function TypeAliases.AddI64(builder, i64)
  builder:PrependInt64Slot(6, i64, 0)
end

function TypeAliases.AddU64(builder, u64)
  builder:PrependUint64Slot(7, u64, 0)
end

function TypeAliases.AddF32(builder, f32)
  builder:PrependFloat32Slot(8, f32, 0.0)
end

function TypeAliases.AddF64(builder, f64)
  builder:PrependFloat64Slot(9, f64, 0.0)
end

function TypeAliases.AddV8(builder, v8)
  builder:PrependUOffsetTRelativeSlot(10, v8, 0)
end

function TypeAliases.StartV8Vector(builder, numElems)
  return builder:StartVector(1, numElems, 1)
end

function TypeAliases.AddVf64(builder, vf64)
  builder:PrependUOffsetTRelativeSlot(11, vf64, 0)
end

function TypeAliases.StartVf64Vector(builder, numElems)
  return builder:StartVector(8, numElems, 8)
end

function TypeAliases.End(builder)
  return builder:EndObject()
end

return TypeAliases