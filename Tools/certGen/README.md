# Certificate Generator

This is a basic self-signed certificate generator for an OPC UA Client.

## Usage
Make a copy of `settings.template.conf` and name it `<env>.conf` (`<env>` is the name of your environment). Adjust your hostname(s) and IP addresses.

### On Linux
Make sure you have *openssl* installed. On Debian based systems you can install *openssl* with `apt install openssl`
Run `createCertificate.sh -e <env>`. All certificates created will be stored in a folder named `<env>`.

### On Windows
Make sure you have *openssl* installed. You can use an *openssl* build from [here](http://wiki.overbyte.eu/wiki/index.php/ICS_Download#Download_OpenSSL_Binaries_.28required_for_SSL-enabled_components.29). Make sure the path to the `openssl.exe` is in your environment variables. You can select another *openssl* build from [here](https://wiki.openssl.org/index.php/Binaries)
Run  `createCertificate.bat -e <env>`. All certificates created will be saved in a folder named `<env>`.