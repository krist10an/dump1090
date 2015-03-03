Vagrant.configure(2) do |config|
  config.vm.box = "ubuntu/trusty32"
  config.vm.network "forwarded_port", guest: 8080, host: 8080

  config.vm.provision "shell", inline: <<-SHELL
    sudo apt-get update
    sudo apt-get install -y build-essential git
    sudo apt-get install -y pkg-config librtlsdr-dev sqlite3 libsqlite3-dev libusb-1.0-0-dev
    sudo apt-get install -y libspatialite-dev proj-bin
  SHELL
end
