
# parsetab.py
# This file is automatically generated. Do not edit.
# pylint: disable=W,C,R
_tabversion = '3.10'

_lr_method = 'LALR'

_lr_signature = 'END MON1 MON2 MON3 MON4 NAME Xstart1 : start ENDstart : start X pname data\n            start : start X pnamestart : X pname datastart : X pnamedata : data MON2 pname\n            | data MON1 pname\n            | data MON3 pname\n            | data MON4 pname\n        data : data MON2\n            | data MON1\n            | data MON3\n            | data MON4\n        data : MON2 pname\n                | MON1 pname\n                | MON3 pname\n                | MON4 pname\n                data : MON2\n                | MON1\n                | MON3\n                | MON4\n                pname : NAME pnamepname : NAME'
    
_lr_action_items = {'X':([0,2,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,],[3,5,-5,-23,-3,-4,-18,-19,-20,-21,-22,-2,-10,-11,-12,-13,-14,-15,-16,-17,-6,-7,-8,-9,]),'$end':([1,4,],[0,-1,]),'END':([2,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,],[4,-5,-23,-3,-4,-18,-19,-20,-21,-22,-2,-10,-11,-12,-13,-14,-15,-16,-17,-6,-7,-8,-9,]),'NAME':([3,5,7,10,11,12,13,16,17,18,19,],[7,7,7,7,7,7,7,7,7,7,7,]),'MON2':([6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,],[10,-23,10,16,-18,-19,-20,-21,-22,16,-10,-11,-12,-13,-14,-15,-16,-17,-6,-7,-8,-9,]),'MON1':([6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,],[11,-23,11,17,-18,-19,-20,-21,-22,17,-10,-11,-12,-13,-14,-15,-16,-17,-6,-7,-8,-9,]),'MON3':([6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,],[12,-23,12,18,-18,-19,-20,-21,-22,18,-10,-11,-12,-13,-14,-15,-16,-17,-6,-7,-8,-9,]),'MON4':([6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,],[13,-23,13,19,-18,-19,-20,-21,-22,19,-10,-11,-12,-13,-14,-15,-16,-17,-6,-7,-8,-9,]),}

_lr_action = {}
for _k, _v in _lr_action_items.items():
   for _x,_y in zip(_v[0],_v[1]):
      if not _x in _lr_action:  _lr_action[_x] = {}
      _lr_action[_x][_k] = _y
del _lr_action_items

_lr_goto_items = {'start1':([0,],[1,]),'start':([0,],[2,]),'pname':([3,5,7,10,11,12,13,16,17,18,19,],[6,8,14,20,21,22,23,24,25,26,27,]),'data':([6,8,],[9,15,]),}

_lr_goto = {}
for _k, _v in _lr_goto_items.items():
   for _x, _y in zip(_v[0], _v[1]):
       if not _x in _lr_goto: _lr_goto[_x] = {}
       _lr_goto[_x][_k] = _y
del _lr_goto_items
_lr_productions = [
  ("S' -> start1","S'",1,None,None,None),
  ('start1 -> start END','start1',2,'p_start','news.py',76),
  ('start -> start X pname data','start',4,'p_start1','news.py',79),
  ('start -> start X pname','start',3,'p_start2','news.py',98),
  ('start -> X pname data','start',3,'p_start3','news.py',108),
  ('start -> X pname','start',2,'p_start4','news.py',125),
  ('data -> data MON2 pname','data',3,'p_data1','news.py',136),
  ('data -> data MON1 pname','data',3,'p_data1','news.py',137),
  ('data -> data MON3 pname','data',3,'p_data1','news.py',138),
  ('data -> data MON4 pname','data',3,'p_data1','news.py',139),
  ('data -> data MON2','data',2,'p_data2','news.py',164),
  ('data -> data MON1','data',2,'p_data2','news.py',165),
  ('data -> data MON3','data',2,'p_data2','news.py',166),
  ('data -> data MON4','data',2,'p_data2','news.py',167),
  ('data -> MON2 pname','data',2,'p_data3','news.py',184),
  ('data -> MON1 pname','data',2,'p_data3','news.py',185),
  ('data -> MON3 pname','data',2,'p_data3','news.py',186),
  ('data -> MON4 pname','data',2,'p_data3','news.py',187),
  ('data -> MON2','data',1,'p_data4','news.py',209),
  ('data -> MON1','data',1,'p_data4','news.py',210),
  ('data -> MON3','data',1,'p_data4','news.py',211),
  ('data -> MON4','data',1,'p_data4','news.py',212),
  ('pname -> NAME pname','pname',2,'p_pname_multi','news.py',229),
  ('pname -> NAME','pname',1,'p_pname','news.py',233),
]
