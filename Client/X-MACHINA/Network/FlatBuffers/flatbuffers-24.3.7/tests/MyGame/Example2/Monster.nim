#[ MyGame.Example2.Monster
  Automatically generated by the FlatBuffers compiler, do not modify.
  Or modify. I'm a message, not a cop.

  flatc version: 24.3.7

  Declared by  : 
  Rooting type : MyGame.Example.Monster ()
]#

import flatbuffers

type Monster* = object of FlatObj
proc MonsterStart*(builder: var Builder) =
  builder.StartObject(0)
proc MonsterEnd*(builder: var Builder): uoffset =
  return builder.EndObject()