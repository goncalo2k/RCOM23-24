<div align='center'>
<img src='https://raw.githubusercontent.com/pedrofvgomes/feup-ipc/main/img/feup.png'>
<h1>RCOM Project 2</h1>

<p>
Gonçalo  Miranda (up202108773)
<br>
Sophie Large (up202303141)

</p>
</div>


## Table of Contents
1. [**Project's idea description**](#1---introduction)
2. [**Related apps / services / systems**](#2---related-apps--services--systems)


## Introduction

bloat bloat boat

## Part 1 - Develoment of the Download Application (app.c and app.h)
## Part 2 - Experiences

### Experience 1 - Configure an IP Network

1. Connect E0 on Tux**Y**3 and Tux**Y**4 on any of the switch ports.

2. Configure Tux**Y**3 and Tux**Y**4, by performing the following commands:
    - Tux**Y**3
    <br>
    <code> ifconfig eth0 up
    <br> ifconfig eth0 172.16.**Y**0.**1**/24
    </code> 
    - Tux**Y**4
    <br>
    <code> ifconfig eth0 up
    <br> ifconfig eth0 172.16.**Y**0.**254**/24
    </code> 
3. Now we can check both the IP and MAC adresses of both computers:
    - TuxY3:
        - IP Address: 172.16.**Y**0.1/24
        - MAC Address: 00:21:5a:61:2f:d4
    - TuxY4:
        - IP Address: 172.16.**Y**0.254/24
        - MAC Address: 00:21:5a:5a:7b:ea

4. We now can test the connectivity between the computers by pinging each other (TuxY3 pings TuxY4 and vice-versa).

    <code>
    TuxY3: ping 172.16.**Y**0.254 -c 10
    <img1>
    <br>
    TuxY4: ping 172.16.**Y**0.1 -c 10
    <img2>
    </code>

    <br>
    The connection is working in case all the packets are correctly recieved.
    <br>
5. Now, we can check the forwarding and the ARP tables. We're doing it on Tux**Y**3.
    <br>
    <code> in: route -n
    <br>out: <img3>
    </code>
    <br>
    <code> in: arp -a
    <br>out: ? (172.16.40.254) at 00:21:5a:5a:7b:ea [ether] on eht0
    </code>
6. We are now deleting the arp table entries in Tux**Y**3.
    
    <code>
    arp -d 172.16.40.254
    </code>

    To test if the deletion occurred correctly, we can perform
    <code>
    in: arp -a
    <br>
    out: 
    </code>
7. Now, we can start Wireshark in Tux**Y**3.eth0 and start capturing packets.

8. After that, we can ping Tux**Y**4 from Tux**Y**3 10 times.
    
    <code>
    ping 172.16.40.254 -c 10
     </code>
<br>

9. Finally, we can stop capturing the packets.

10. When saving the logs and analysing them, we've got the following:

<img4>

# TODO
#### Questions

1. What are the ARP packets and what are they used for?

Os pacotes ARP (*Address Resolution Protocol*) são pacotes que servem para estabelecer a ligação entre um IP e um endereço MAC.

2. What are the MAC and IP addresses of ARP packets and why?

São enviados em broadcast 2 endereços IP: o da máquina de destino (172.16.50.254, Tux54) e o da máquina de origem (172.16.50.01, Tux 53) no mesmo pacote ARP. Como resposta o Tux54 envia um pacote ARP para o Tux53 que contém o seu endereço MAC, 00:21:5a:c3:78:70.

3. What packets does the ping command generate?

Enquanto não obtém o endereço MAC da máquina de destino, o *ping* gera pacotes ARP. Após fazer a ligação do endereço IP ao respetivo MAC, gera pacotes do tipo ICMP (*Internet Control Message Protocol*), para transferência de informação.

4. What are the MAC and IP addresses of the ping packets?

Os endereços IP e MAC usados em pacotes ICMP são os das máquinas em comunicação: Tux53 e Tux54. 

5. How to determine if a receiving Ethernet frame is ARP, IP, ICMP?

A designação do protocolo de cada frame está presente na coluna "Protocol" da captura no Wireshark. Por norma o protocolo é indicado nos primeiros 2 bytes da frame, no cabeçalho.

6. How to determine the length of a receiving frame?

Existe uma coluna no Wireshark que mostra o tamanho, em bytes, das frames. Por norma o cabeçalho de cada frame contém essa informação.

7. What is the loopback interface and why is it important?

É uma interface virtual que é sempre alcançável desde que pelo menos uma das interfaces IP do *switch* esteja operacional. Com isto, é possível verificar periodicamente se as ligações da rede estão configuradas corretamente.

### Experience 2 - Implement two bridges in a switch

1. Connect E0 on Tux**Y**2 to any of the switch ports. After that,configure Tux**Y**2, by performing the following commands:
    <br>
    <code> ifconfig eth0 up
    <br> ifconfig eth0 172.16.**Y**1.**1**/24
    </code> 

2. Now, we can creating two bridges in the switch, using the following commands on GTKTerm.
    - Bridge**Y**0:
        
        <code>
        /interface bridge add name=bridge**y**0
        </code>
    - Bridge**Y**1:
        
        <code>
        /interface bridge add name=bridge**y**1
        </code>

3. Afterwards, we can remove the ports where Tux**Y**2, Tux**Y**3, and Tux**Y**4 are connected from the default bridge and add them to the corresponding ports to bridge**Y**0 and bridge**Y**1.
    - Removing the original ports:
        
        <code>
        /interface bridge port remove [find interface=ether1] 
        <br>
        /interface bridge port remove [find interface=ether2] 
        <br>
        /interface bridge port remove [find interface=ether3]
        </code>

    - Adding the current ports:
        
        <code>
        /interface bridge port add bridge=bridge50 interface=ether1
        <br>
        /interface bridge port add bridge=bridge50 interface=ether2 
        <br>
        /interface bridge port add bridge=bridge51 interface=ether3
        </code>

<img5>
4. Now, we should switch to Tux**Y**3 and start monitoring the packets.

5.Ping Tux**Y**4 and Tux**Y**2 from Tux**Y**3.

### Experience 3 - Configure a router in Linux

### Experience 4 - Configure a Commercial Router and Implement NAT

### Experience 5 - DNS

### Experience 6 - TCP Connections

## Conclusions

bloat bloat bloat

## Annexes