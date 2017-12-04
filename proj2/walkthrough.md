# Configurar rede final

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
   * `configure terminal`  + `vlanY0` + `exit` + `vlanY1` + `exit` (`exit` sai um nível, `end` sai todos, neste caso o `exit` evita novo `configure terminal`)  - criar as vlans (não fazer exit no fim para o próximo passo)
   * [só se necessário: `configure terminal`] + `interface fastethernet 0/1` + `switchport mode access` + `switchport access vlanY0`(0 é o bloco do switch (só temos 1), 1 é a porta a qual o cabo do tuxY1 deve ligar no switch) 
   * repetir o comando acima para (vlanY0  + 0/2; vlanY1 + 0/13; vlanY1 + 14; vlanY1 + 15) (13, 14, 15 são portas fisicamente relevantes, podiam ser quaisquer outras) + `end`
   
 3. Configurar IPs nos tuxs
  * Mudar para o tuxYX (X em [1, 4, 2]) e fazer o comando seguinte com o IP correto
  * `ifconfig eth0 172.16.Y0.1/24` (/24 'e a subnet - quantos bits do IP estão fora da subnet interna (24 = 3 * 8, ie, 172.16.Y0.QQ pertence à mesma rede))
    * tuxY0 - `ifconfig eth0 172.16.Y0.1/24`
    * tuxY4 - `ifconfig eth0 172.16.Y0.254/24`
    * tuxY4 - `ifconfig eth1 172.16.Y1.253/24`
    * tuxY2 - `ifconfig eth0 172.16.Y1.1/24`
  * Ligar o router
  
   
   
   * [TODO]copy running-config flash:<turma-nome1-nome2-nome3>
 
Vlan 30 - tux31 (172.16.30.1/24 | eth0) + tux34(172.16.30.254/24 | eth0)
Vlan 31 - tux31 (172.16.30.1/24 | eth0) + tux34(172.16.30.254/24 | eth0)

