configFileDir = os.getenv("HOME") .. "/.config/musicritty/config"

configs = {
  dir = os.getenv("HOME") .. "/.config/musicritty/config",
  libs
}

function test()
  print("Hello, World!")
end

function openConfig()
  file = io.open(configs.dir, "r")
  for line in io.lines(configs.dir) do
    if string.find(line, "libs") then
    end
  end
  io.close(file)
end

openConfig()

function newConfig()
end
