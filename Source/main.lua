import "CoreLibs/ui"
import "CoreLibs/graphics"
import "CoreLibs/object"

local Menu <const> = playdate.getSystemMenu()
local Graphics <const> = playdate.graphics
local Display <const> = playdate.display
local idle = 0
local running = 1
local state = idle

table.filter = function(t, filterIter)
    local out = {}

    for k, v in pairs(t) do
        if filterIter(v, k, t) then out[k] = v end
    end

    return out
end

local function ends_with(str, ending)
    return ending == "" or str:sub(-#ending) == ending
end

local menuOptions <const> = table.filter(
        playdate.file.listFiles("roms"),
        function(o)
            return ends_with(o, ".ch8")
        end)

local listview = playdate.ui.gridview.new(0, 40)
listview:setNumberOfRows(#menuOptions)
listview:setCellPadding(0, 0, 8, 0)
listview:setContentInset(8, 8, 0, 0)

function listview:drawCell(section, row, column, selected, x, y, width, height)
    if selected then
        Graphics.fillRoundRect(x, y, width, height, 4)
        Graphics.setImageDrawMode(Graphics.kDrawModeFillWhite)
    else
        Graphics.setImageDrawMode(Graphics.kDrawModeCopy)
    end
    Graphics.drawTextInRect("*"..menuOptions[row]:gsub(".ch8", "").."*", x+10, y+(11), width, height, nil, "...", kTextAlignment.left)
end

local showFPS = false
local function addMenuButton()
    Menu:addMenuItem("exit", function()
        state = idle
        playdate.inputHandlers.push(inputHandlers)
        Menu:removeAllMenuItems()
        Graphics.clear()
        listview:drawInRect(0, 0, 400, 240)
    end)
    Menu:addCheckmarkMenuItem("inverted", false, function(inv)
        Display.setInverted(inv)
    end)
    Menu:addCheckmarkMenuItem("show fps", showFPS, function(show)
        showFPS = show
    end)
end

local inputHandlers

inputHandlers = {
    upButtonUp = function()
        listview:selectPreviousRow(true)
    end,
    downButtonUp = function()
        listview:selectNextRow(true)
    end,
    AButtonUp = function()
        playdate.inputHandlers.pop()
        state = running
        loadRom("roms/"..menuOptions[listview:getSelectedRow()])
        addMenuButton()
    end
}

playdate.inputHandlers.push(inputHandlers)

function playdate.update()
    if state == running then
        emulate()
    end
    
    if listview.needsDisplay == true and state == idle then
        Graphics.clear()
        listview:drawInRect(0, 0, 400, 240)
        playdate.timer:updateTimers()
    end
        
    if showFPS then
        playdate.drawFPS(0, 0)
    end
end