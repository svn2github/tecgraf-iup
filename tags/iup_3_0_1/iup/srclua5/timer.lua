------------------------------------------------------------------------------
-- Timer class 
------------------------------------------------------------------------------
local ctrl = {
  nick     = "timer",
  parent   = WIDGET,
  creation = "",
  callback = {
    action_cb = "", 
  },
} 

function ctrl.createElement(class, param)
  return Timer()
end
   
iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
