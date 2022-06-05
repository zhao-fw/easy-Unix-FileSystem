目标程序：UnixFileSystem.exe

模拟的磁盘文件：myDisk.img

Jerry测试文件：test、abc

实验报告：1952529-赵一凡.pdf
ReadMe文件：ReadMe.txt
图片文件：photo.png


类Unix文件系统的使用方法：
[man]
Command       :  man - 显示帮助手册
Usage         :  man [命令]
Parameter     :  [命令] 如下：
                 man          :  手册
                 fformat      :  格式化
                 exit         :  退出
                 mkdir        :  新建目录
                 cd           :  改变目录
                 ls           :  列出目录及文件
                 create       :  新建文件
                 delete       :  删除文件
                 open         :  打开文件
                 close        :  关闭文件
                 seek         :  移动读写指针
                 write        :  写入文件
                 read         :  读取文件

[fformat]
Command       :  fformat - 进行文件系统格式化
Description   :  将整个文件系统进行格式化，即清空所有文件及目录!
Usage         :  fformat
Parameter     :  无

[create]
Command       :  create - 新建文件
Description   :  新建一个文件。
Usage         :  create <文件名> <选项>
Parameter     :  <文件名>
                 <选项> -r 只读属性
                 <选项> -w 只写属性
                 <选项> -rw == -r -w 读写属性

[read]
Command       :  read -读取文件
Description   :  从一个已经打开的文件中读取。
Usage         :  read <file descriptor> [-o <OutFileName>] <size>
Parameter     :  <file descriptor> open返回的文件描述符
                 [-o <OutFileName>] -o 指定输出方式为文件，文件名为 <OutFileName> ,不指定为shell
                 <size> 指定读取字节数，大小为 <size>

[exit]
Command       :  exit - 退出文件系统
Description   :  该指令实现程序的正确退出，保证磁盘文件得到更新。
Usage         :  exit
Parameter     :  无

[ls]
Command       :  ls - 列出当前目录内容
Description   :  列出当前目录中包含的文件名或目录名。
Usage         :  ls
Parameter     :  无

[mkdir]
Command       :  mkdir - 建立目录
Description   :  新建一个目录。
Usage         :  mkdir <目录名>
Parameter     :  <目录名> 可以是相对路径，也可以是绝对路径

[delete]
Command       :  delete - 删除文件
Description   :  删除一个文件。
Usage         :  delete <文件名>
Parameter     :  <文件名>

[cd]
Command       :  cd - 改变当前目录
Description   :  改变当前工作目录。
Usage         :  cd <目录名>
Parameter     :  <目录名>可以从当前目录出发，也可以从根目录出发；

[open]
Command       :  open - 打开文件
Description   :  打开一个文件。若要进行文件的读写必须先open。
Usage         :  open <文件名> <选项>
Parameter     :  <文件名>
                 <选项> -r 只读属性
                 <选项> -w 只写属性
                 <选项> -rw == -r -w 读写属性

[close]
Command       :  close - 关闭文件
Description   :  关闭一个文件。
Usage         :  close <file descriptor>
Parameter     :  <file descriptor> 文件描述符

[seek]
Command       :  seek - 写入文件
Description   :  移动某个打开文件的读写指针位置
Usage         :  seek <file descriptor> <offset> <origin>
Parameter     :  <file descriptor> open返回的文件描述符
                 <offset> 指定从 <origin> 开始的偏移量 可正可负
                 <origin> 指定起始位置 可为0(SEEK_SET), 1(SEEK_CUR), 2(SEEK_END)

[write]
Command       :  write - 写入文件
Description   :  写入一个已经打开的文件中。
Usage         :  write <file descriptor> <InFileName> <size>
Parameter     :  <file descriptor> open返回的文件描述符
                 <InFileName> 指定写入内容为文件InFileName中的内容
                 <size> 指定写入字节数，大小为 <size>