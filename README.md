# f767zi_micro-ros_test

## 概要
このプロジェクトでは、STMのf767ziボードを使用して、micro-rosを体験してみようというものです。STM32CubeIDE for VSCodeとSTM32CubeMXを使用しました。

## 環境
- **ボード**: NUCLEO-F767ZI
- **ROS2 distro**: ROS2 Humble
- **OS**: Ubuntu 22.04 LTS

## 手順

### 0. 諸々用意
以下のものを用意してください。
- **F767ZIボード**
- **micro-b to type-A**
- **LANケーブル**
- **ros2 humbleが入ってるPC**

### 1. 必要なVSCode拡張機能のインストール
以下の拡張機能をVSCodeにインストールします。
- **CMake Tools**
- **STM32CubeIDE for Visual Studio Code**

### 2. micro-ros-agentの用意（任意）
以下のコマンドを実行してください。
``` bash
git clone https://github.com/micro-ROS/micro_ros_setup.git
cd micro_ros_setup
ros2 run micro_ros_setup create_agent_ws.sh
ros2 run micro_ros_setup build_agent.sh
source install/local_setup.bash
```
## <span style="color: red;">注意：docker版もあるからね<span>

### 3. リポジトリクローン
以下のコマンドを実行してください。
```bash
git clone https://github.com/kai-1208/f767zi_micro-ros_test.git
code ./f767zi_micro-ros_test
```

### 4. Debug
VSCodeでSTM32CubeIDE for VSCodeを開いて、Set Up STM32Cube projectを選択 → Save and closeを選択 → Debug

### 5. CMake build
CMake toolsを開いて、Project OutlineのBuild All Projectsを選択

### 6. Run and Debug
f767ziにmicro-b to type-aで接続<br>
Run and Debugを選択 → ST-LINK GDB serverを選択

### 7. ip設定
lanケーブルを接続<br>
ipを以下のように設定してください。
- #### ubuntuの場合
- Network → Wired → IPv4<br>
- **IPv4 Method**: Manual<br>
- **Addresses**: 
  - **Address**: 192.168.1.10
  - **Netmask**: 255.255.255.0
  - **Gateway**: 192.168.1.1

Applyしたあと、一旦Wiredを無効化し、再度有効化してください。(これで設定が反映されます。)

### 8. pingテスト
以下のコマンドを実行してください。
```bash
ping 192.168.1.100
```
以下のように表示されれば成功です。
```bash
:~$ ping 192.168.1.100
PING 192.168.1.100 (192.168.1.100) 56(84) bytes of data.
64 bytes from 192.168.1.100: icmp_seq=1 ttl=255 time=3.09 ms
64 bytes from 192.168.1.100: icmp_seq=2 ttl=255 time=2.41 ms
```

### 9. agentの起動
以下のコマンドを実行してください。
```bash
# 2. micro-ros-agentの用意にてgit cloneした場合は、下のコマンドを実行してください。
ros2 run micro-ros-agent micro-ros-agent udp4 --port 8888
# docker版は下のコマンドを実行してください。
sudo docker run -it --rm -p 8888:8888/udp microros/micro-ros-agent:humble udp4 --port 8888
```
実行したら、f767ziボード上のリセットボタンを押して以下のように表示されたら成功です。
```bash
[1779092672.857217] info     | UDPv4AgentLinux.cpp | init                     | running...             | port: 8888
[1779092672.857558] info     | Root.cpp           | set_verbose_level        | logger setup           | verbose_level: 4
[1779092678.193843] info     | ProxyClient.cpp    | ProxyClient              | session hard timeout enabled | client_key: 0x1CA978CC, timeout: 1000 ms
[1779092678.193919] info     | Root.cpp           | create_client            | create                 | client_key: 0x1CA978CC, session_id: 0x81
[1779092678.194053] info     | SessionManager.hpp | establish_session        | session established    | client_key: 0x1CA978CC, address: 192.168.1.100:12276
[1779092678.208487] info     | ProxyClient.cpp    | create_participant       | participant created    | client_key: 0x1CA978CC, participant_id: 0x000(1)
[1779092678.212659] info     | ProxyClient.cpp    | create_topic             | topic created          | client_key: 0x1CA978CC, topic_id: 0x000(2), participant_id: 0x000(1)
[1779092678.216106] info     | ProxyClient.cpp    | create_subscriber        | subscriber created     | client_key: 0x1CA978CC, subscriber_id: 0x000(4), participant_id: 0x000(1)
[1779092678.219808] info     | ProxyClient.cpp    | create_datareader        | datareader created     | 
```

### 10. LD1を点灯・消灯
ここまできたらあとはtopicが存在しているのかの確認、実際にf767ziボード上のLD1を点灯消灯させていきます。以下のコマンドを実行してください。
```bash
ros2 topic list
```
以下のように表示されたら成功です。
```bash
/led_topic
```
では、以下のコマンドを実行してください。f767ziボード上のLD1が点灯・消灯したら成功です。
```bash
# 点灯
ros2 topic pub /led_topic std_msgs/msg/Int32 "{data: 1}"
# 消灯
ros2 topic pub /led_topic std_msgs/msg/Int32 "{data: 0}"
```

## おまけという名の本命
micro-ROSとは、簡単にいえば **「マイコン上でROS2を動作させるためのフレームワーク」** といった感じです。<br>
通常、ROS2は、LinuxなどのOSが動くPCで動作するように設計されているのですが、マイコンではメモリや処理能力が限られているため、そのままではROS2を動かすことはできません。<br>
そこでmicro-ROSは、マイコンでもROS2を使用したいという思いから開発されました。<br>

### 主な特徴

#### 1. マイコンをROS2の「ノード」として扱える
マイコン自体がROS2の正規のノードとして、topicのpub/sub、serviceなどが直接利用できます。

#### 2. 非常に軽量
標準のROS2が使用する通信プロトコル（DDS）を、マイコン向けの軽量版の「Micro XRCE-DDS」に置き換えています。これにより、RAMが数十〜数百KB程度のマイコンでも動作可能です。

#### 3. RTOSに対応
FreeRTOSやZephyrといった、RTOS上で動作します。

#### 4. 多様な通信方法
シリアル（USB）、UDP（Wi-Fi/Ethernet）、CAN、Bluetoothなど、様々な物理層で通信可能です。

### micro-ROSの仕組み
micro-ROSは主に以下の2つの要素で構成されています。

- **micro-ros client**（マイコン側）<br>
マイコン上で動作するライブラリ。ここでノードを作成し、データの送受信を行います。
- **micro-ros agent**（PC側）<br>
マイコンからの軽量な通信を受け取り、標準的なROS2（DDS）の通信に変換して仲介する役割を担います。

### 通信の流れ
```
[マイコン(Client)] <---> [PC(Agent)] <---> [他のROS2ノード]
```
### なぜ使うのか？（メリット）
- **開発の統一**: ロボット全体のシステム（PC上のAI処理からマイコン上のモーター制御まで）を、すべてROS 2の作法で統一して記述できます。
- **分散処理**: 複雑な計算はPCで行い、高速な応答が必要な物理制御はマイコンで行うといった構成が簡単に作れます。
- **標準化**: 独自プロトコルを設計することなく、標準的なROS 2のツール（Rviz2での可視化やrqtでのデバッグ）をそのままマイコンのデータに対して使えます。
