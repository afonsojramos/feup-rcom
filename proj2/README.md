# Configurar rede final


 ![Apontamentos deixados no quadro](https://imgur.com/du4Ui8d.png)


 1. Ligar cabos 
   * Ligar um tux ao switch (por exemplo o tuxY1)
   * Ligar cada um dos tux às respetivas portas (neste caso usamos: tuxY1+eth0 - port 1, tuxY4+eth0 - port 2, tuxY4+eth1 - port 13, tuxY2+eth0 - port 14, router - + port 15)
    
 2. Configurar Vlans (Y0 e Y1) no switch
   * Mudar para tux ligado ao switch (tuxY1)
   * abrir gtkterm
   * `enable` + password: `8nortel` - iniciar sessão no switch por serial connection
   * `show vlan brief` mostra as vlans criadas
   * `configure terminal` + `no vlan 2-4094` + `exit` - para fazer apagar as configuraçes do switch
   * `copy flash:gnu3-clean startup-config` + `reload` - para repor as configuraçes de origem [se não funcionar porque apagaram o ficheiro de configuração, esquecer este passo]
   * `configure terminal`  + `vlan Y0` + `exit` + `vlan Y1` + `exit` (`exit` sai um nível, `end` sai todos, neste caso o `exit` evita novo `configure terminal`)  - criar as vlans (não fazer exit no fim para o próximo passo)
   * [só se necessário: `configure terminal`] + `interface fastethernet 0/1` + `switchport mode access` + `switchport access vlanY0`(0 é o bloco do switch (só temos 1), 1 é a porta a qual o cabo do tuxY1 deve ligar no switch) 
   * repetir o comando acima para (vlanY0  + 0/2; vlanY1 + 0/13; vlanY1 + 14; vlanY1 + 15) (13, 14, 15 são portas fisicamente relevantes, podiam ser quaisquer outras) + `end`
   
  ###### Os pontos dos passos 3. e 4. referentes à execução de comandos em linux podem ser substituidos pela execução dos ficheiros configTuxYX.sh respetivos.
  
 3. Configurar IPs (tuxs + router)
  * Mudar para o tuxYX (X em [1, 4, 2]) e fazer o comando seguinte com o IP correto
  * `service networking restart` (para repor as definiçes padrão)
  * `ifconfig eth0 172.16.Y0.1/24` (/24 'e a subnet - quantos bits do IP estão fora da subnet interna (24 = 3 * 8, ie, 172.16.Y0.QQ pertence à mesma rede))
    * tuxY1 - `ifconfig eth0 172.16.Y0.1/24`
    * tuxY4 - `ifconfig eth0 172.16.Y0.254/24`
    * tuxY4 - `ifconfig eth1 172.16.Y1.253/24`
    * tuxY2 - `ifconfig eth0 172.16.Y1.1/24`
  * Ligar o router
  * Trocar o cabo que vinha do tuxY1 (ligado ao switch) e ligar ao router (porta ao lado)
  * Abrir gtkterm
  * username `root` + password `8nortel`
  * [opcional] `show running-config` para visualizar configuração atual
  * `conf t` + `interface gigabitethernet 0/0` (0 é o bloco/0 é a gigaethernet port no switch) + `ip address 172.16.Y1.254 255.255.255.0` (o 255.255.255.0 é outra forma de especificar a netmask) + `no shutdown` + `ip nat inside` + `exit` (sem fazer `end` para o próximo)
  * `interface gigabitethernet 0/1` (0 é o bloco/1 é a gigaethernet port no switch) + `ip address 172.16.1.Y9 255.255.255.0` + `no shutdown` + `ip nat outside` + `exit` (é `outside` e não `inside` para ligar ao exterior)
  * `ip nat pool ovrld 172.16.1.Y9 172.16.1.Y9 prefix 24`
  * `ip nat inside source list 1 pool ovrld overload`
  * `access-list 1 permit 172.16.Y0.0 0.0.0.7` - para a vlanY0
  * `access-list 1 permit 172.16.Y1.0 0.0.0.255` - para a vlanY1
  * `ip route 0.0.0.0 0.0.0.0 172.16.1.254`
  * `ip route 172.16.Y0.0 255.255.255.0 172.16.Y1.253` - Se o router precisar de mandar para a rede Y30, que tem a máscara `255.255.255.0`, mandar antes para `172.16.Y1.253` e este sabe o que fazer
  * `end`
  
 4. Configurar Rotas
  * Apagar as rotas (`route del -net <> gw <> netmask <>`)(<> = destination, gateway, netmask)[não deve ser preciso]
  * `route add -net 172.16.Y1.0 gw 172.16.Y0.254 netmask 255.255.255.0` - dizer ao tuxY1 como aceder ao tuxY2 (pelo tuxY4) e ao router (ping de tuxY1 para tuxY2 deve funcionar)
  * Mudar para tuxY4 (para lhe dizer para fazer forward correto dos pacotes que não so para ele em vez de os descartar)
   * `echo 1> /proc/sys/net/ipv4/ip_forward`
   * `echo 0> /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts`
   * `route -n` ver rotas, não deve ser preciso mexer
  * Mudar para o tuxY2
   * `route add -net 172.16.30.0 gw 172.16.31.253 netmask 255.255.255.0`
   * ping do 2 para 1 já deve funcionar [passo era opcional]
 
 5. Guardar configuração do switch
 * Ligar o tuxY1 ao switch
 * `copy running-config flash:<turma-nome1-nome2-nome3>` - a cópia fica feita (para nós: **danafig**)
 
 6. Apresentar FTP client na rede configurada :muscle:
 
 **NOTA:** Entregar relatório até dia 22/12

