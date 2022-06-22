git = {}

git.clone_repo = function(url, dst)
    if (not os.isdir(path.getabsolute(dst))) then
        os.execute("git clone " .. url .. " " .. dst)
    end
end
