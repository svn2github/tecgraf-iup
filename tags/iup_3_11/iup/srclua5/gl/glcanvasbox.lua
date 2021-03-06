------------------------------------------------------------------------------
-- GLCanvasBox class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "glcanvasbox",
  parent = iup.BOX,
  creation = "-",
  funcname = "GLCanvasBox",
  include = "iupglcontrols.h",
  subdir = "gl",
  callback = {
    action = "ff",
    swapbuffers_cb = "",
  },
}

function ctrl.append(handle, elem)
  iup.Append(handle, elem)
end

function ctrl.createElement(class, param)
   return iup.GLCanvasBox()
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
