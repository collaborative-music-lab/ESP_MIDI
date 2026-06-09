import espnow, wifi
import time

class ESPNowPeerManager:
    """
    Manages ESP-NOW peer discovery and communication.
    
    Features:
    - Broadcasts device MAC address and metadata
    - Auto-discovers peers from broadcasts
    - Targets peers by index or name
    - Handles duplicate device names with numeric suffixes
    """
    
    def __init__(self, device_name: str, device_type: str, broadcast_interval: float = 2.0):
        """
        Initialize peer manager.
        
        Args:
            device_name: Base name for this device (e.g., "robin")
            device_type: Type of device (e.g., "clock", "sequencer", "keyboard")
            broadcast_interval: Seconds between broadcasts (default 2.0)
        """
        # Initialize WiFi, set to channel 6 where ESP-NOW lives
        wifi.radio.start_ap(" ", "", channel=6, max_connections=0)
        wifi.radio.stop_ap()
        self.mac_address = wifi.radio.mac_address
        print(f"My MAC: {self.mac_address.hex()}")


        self.device_name = device_name
        self.device_type = device_type
        self.broadcast_interval = broadcast_interval
             
        # Initialize ESP-NOW
        self.espnow = espnow.ESPNow()
        
        # Peer storage: {'name': str, 'type': str, 'peer':peer}
        self.peers = []
        self._peer_names = {} # Track names for duplicate detection, {base_name: [count]}
        self.broadcast_mac = b'\xff\xff\xff\xff\xff\xff'
        self.broadcast_peer = espnow.Peer(mac= b'\xff\xff\xff\xff\xff\xff',channel=6)
        self.espnow.peers.append(self.broadcast_peer)
        
        # Timing
        self._last_broadcast = time.time()
        
        print(f"[{device_name}] Initialized. MAC: {self._mac_to_string(self.mac_address)}")
    
    @staticmethod
    def _mac_to_string(mac: bytes) -> str:
        """Convert MAC address bytes to readable string."""
        return ':'.join(f'{b:02x}' for b in mac)
    
    @staticmethod
    def _mac_to_bytes(mac_str: str) -> bytes:
        """Convert MAC address string to bytes."""
        return bytes(int(x, 16) for x in mac_str.split(':'))
    
    def _generate_peer_name(self, base_name: str) -> str:
        """Generate unique name with numeric suffix if duplicates exist."""
        if base_name not in self._peer_names:
            self._peer_names[base_name] = 1
            return f"{base_name}01"
        
        # Increment counter for this name
        self._peer_names[base_name] += 1
        count = self._peer_names[base_name]
        return f"{base_name}{count:02d}"
    
    def broadcast_presence(self, force: bool = False) -> bool:
        """
        Broadcast this device's presence to all peers.
        
        Args:
            force: Ignore interval and broadcast immediately
            
        Returns:
            True if broadcast sent, False if interval not elapsed
        """
        now = time.time()
        if not force and (now - self._last_broadcast) < self.broadcast_interval:
            return False
        
        # Broadcast message: [MAC, Name, Type]
        broadcast_data = f"{self._mac_to_string(self.mac_address)}|{self.device_name}|{self.device_type}".encode()
        
        try:
            self.espnow.send(broadcast_data, self.broadcast_peer)
            self._last_broadcast = now
            return True
        except Exception as e:
            print(f"[ERROR] Failed to broadcast: {e}")
            return False
    
    def add_peer(self, mac_address: bytes, name: str, device_type: str) -> str:
        """
        Add or update a peer.
        
        Args:
            mac_address: Peer's MAC address (bytes)
            name: Peer's base name
            device_type: Type of peer device
            
        Returns:
            Generated peer name with numeric suffix
        """
        mac_str = self._mac_to_string(mac_address)
        
        # Check if peer already exists
        if mac_address in self.peers:
            return self.peers[mac_address]['name']
        
        # Generate unique name
        peer_name = self._generate_peer_name(name)
        
        # # Create espnow.Peer object
        peer_obj = espnow.Peer(mac=mac_address, channel=self.channel)
        self.espnow.peers.append(peer_obj)
        
        # Add to peers array
        peer_index = len(self.peers)
        self.peers.append({
            'name': peer_name,
            'peer': peer_obj,
            'type': device_type,
            'mac': mac_address
        })
        
        print(f"[PEER ADDED] {peer_name} ({device_type}) - {mac_str}")
        return peer_name
    
    
    def handle_incoming(self):
        """
        Process incoming ESP-NOW messages (broadcasts and data).
        Uses event-based reading: espnow.read()
        
        Returns:
            Packet object if regular data, True if peer discovery, False if nothing
        """
        if not self.espnow:  # Check if event exists
            return False
        
        packet = self.espnow.read()  # Read the packet
        
        if packet is None:
            return False
        
        try:
            # packet.msg contains the message bytes
            # packet.mac contains the sender's MAC address
            data = packet.msg.decode('utf-8')
            sender_mac = packet.mac
            
            parts = data.split('|')
            
            if len(parts) == 3:
                # Broadcast message (peer discovery)
                mac_str, name, device_type = parts
                mac_bytes = self._mac_to_bytes(mac_str)
                
                # Don't add ourselves
                if mac_bytes == self.mac_address:
                    return False
                
                self.add_peer(mac_bytes, name, device_type)
                return True
            else:
                # Regular message - return packet for application to handle
                return packet
        
        except Exception as e:
            print(f"[ERROR] Failed to parse message: {e}")
            return False
    
    def broadcast(self, message: str) -> int:
        """
        Broadcast message to all known peers.
        
        Args:
            message: Message to send
            
        Returns:
            True if sent successfully, False otherwise
        """
        msg_bytes = message.encode('utf-8')
        
        for peer_obj in self.peers:
            try:
                self.espnow.send(msg_bytes, peer_obj['peer'])
                return True
            except Exception as e:
                print(f"[ERROR] Failed to send to {peer_obj['name']}: {e}")
                return False
        
        return False
    
    def send_to_peer(self, target, message: str) -> bool:
        """
        Send message to specific peer by name or index.
        
        Args:
            target: Peer name (string) or peer index (int)
            message: Message to send
            
        Returns:
            True if sent successfully, False otherwise
        """
        peer_obj = self._resolve_peer(target)
        
        if peer_obj is None:
            print(f"[ERROR] Peer not found: {target}")
            return False
        
        try:
            msg_bytes = message.encode('utf-8')
            self.espnow.send(msg_bytes, peer_obj['peer'])
            print(f"[SENT] To {peer_obj['name']}: {message}")
            return True
        except Exception as e:
            print(f"[ERROR] Failed to send message: {e}")
            return False
    
    def _resolve_peer(self, target):
        """
        Resolve peer name or index to peer object.
        
        Args:
            target: Peer name (string) or index (int)
            
        Returns:
            Peer object dict or None if not found
        """
        if isinstance(target, int):
            # Search by index
            if 0 <= target < len(self.peers):
                return self.peers[target]
        else:
            # Search by name
            for peer in self.peers:
                if peer['name'] == target:
                    return peer
        
        return None
    
    def get_peers(self) -> list:
        """
        Get list of all peers.
        
        Returns:
            List of peer dictionaries with index, name, type, and MAC
        """
        peers_list = []
        for index, peer in enumerate(self.peers):
            peers_list.append({
                'index': index,
                'name': peer['name'],
                'type': peer['type'],
                'mac': self._mac_to_string(peer['mac'])
            })
        return peers_list
    
    def print_peers(self):
        """Print formatted peer list."""
        print("\n[PEERS]")
        if not self.peers:
            print("  No peers discovered yet")
            return
        
        for peer in self.get_peers():
            print(f"  [{peer['index']}] {peer['name']:15} ({peer['type']:12}) {peer['mac']}")
        print()



# Initialize manager
manager = ESPNowPeerManager(
    device_name="robin",
    device_type="clock",
    broadcast_interval=1.0
)

# Main loop
try:
    while True:
        # Broadcast presence periodically
        manager.broadcast_presence()
        
        # Listen for incoming messages and peer discoveries
        manager.handle_incoming()
        
        # Send to specific peer by index
        if len(manager.peers) > 0:
            manager.send_to_peer(0, "Hello peer!")
        
        # Send to specific peer by name
        if len(manager.peers) > 1:
            manager.send_to_peer("robin02", "Hi robin02!")
        
        # Broadcast to all peers
        if len(manager.peers) > 0:
            manager.broadcast("Broadcast message!")
        
        # Print peers periodically
        manager.print_peers()
        
        time.sleep(5)

except KeyboardInterrupt:
    print("Stopping...")