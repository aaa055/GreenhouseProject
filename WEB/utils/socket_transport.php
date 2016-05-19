<?php
if (!defined('IN_SERVER')) { exit; }

class SocketTransport
{
  var $address;
  var $port;
  var $sock;
  var $timeout;
  
  
  function __construct()
  {
    $this->sock = 0;
    $this->port = 1975;
    $this->address = "";
    $this->timeout = 5;
  }
  
  function __destruct()
  {
    $this->close();
  }

  //
  // open
  //
  function open($addr, $port=1975, $tmio=5)
  {
    $this->close();
    
    $this->address = $addr;
    $this->port = $port;
    $this->timeout = $tmio;
    
    $this->sock = @fsockopen($this->address, $this->port,$errno, $errstr,$this->timeout);

    return $this->sock;
  }
    
  //
  // Other base methods
  //
  function close()
  {
    if($this->sock)
    {
      @fclose($this->sock);
      $this->sock = 0;
    }
    
    return true;
  }

  //
  // Base query method
  //
  function ctget($query = '')
  {
      if(!$this->sock)
        return false;
        
     @fwrite($this->sock, 'CTGET=' . $query . "\r\n");
               
     return @fgets($this->sock, 1024);
  }
  //
  //  Base set method
  //
  function ctset($query = '')
  {
      if(!$this->sock)
        return false;
        
     @fwrite($this->sock, 'CTSET=' . $query . "\r\n");
               
     return @fgets($this->sock, 1024);
  }


 

} // class SocketTransport

?>