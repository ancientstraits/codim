function sleep(time)
    os.execute('sleep '..time)
end

print('a')
sleep(0.5)
print('b')
sleep(1)
print('c')
