# Troubleshooting

## Missing DLLs (only Windows binary)

In case DLLs are missing, those are most likely from the Visual C++ Redistributable package. Those can be downloaded [here](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170).

## Using the Gateway with Firewall and TLS Inspection

### Introduction

This documentation describes how to use the gateway in an environment with a firewall and TLS Inspection. TLS Inspection, also known as SSL Inspection, is a security mechanism that allows the firewall to inspect the content of encrypted traffic. During TLS Inspection, the firewall exchanges the certificates to decrypt, inspect, and re-encrypt the traffic before forwarding it to its final destination.
Therefore, the CA certificates of the firewall must be stored to ensure that the firewall can correctly inspect and forward the traffic.

### Steps to Follow

- **Check if TLS Inspection is Present**

First, you should check if TLS Inspection is present. Normally, when you access a TLS page (<https://>) in your Corporate Browser, you should not see the certificate of the page, but that of your firewall with TLS Inspection.
For the Dashboard viste <https://umati.app>

- **Obtain the CA Certificates**

If TLS Inspection is present, you need to obtain the CA certificates of the firewall. You can then simply download the certificate in the browser.

- **Add the CA Certificates to the `cacert.pem` File or as a Separate File**

After you have obtained the CA certificates, you have two options:

    - Add the CA certificate as a text block to the `cacert.pem` file in the cert directory.

    - Place the CA certificate separately in a directory as a `.pem` or `.crt` file and explicitly specify the path in `CaCertPath` in the configuration.

- **Adjust the Configuration**

If you have placed the certificate separately as a `.pem` or `.crt` file in a directory, you need to adjust the configuration to specify the path to the CA certificate file. Here are the corresponding fields:

    "CaCertPath":"", // Path to the CA certificate file, only to be set if the certificate is placed separately as a `.pem` or `.crt` file in a directory
    "CaTrustStorePath": "" // Path to the CA certificate file, only to be set if advised

Please note that these fields should only be set if explicitly advised.

### Conclusion

After these steps have been carried out, the gateway should be able to be used successfully in an environment with a firewall and TLS Inspection. If you have further questions or concerns, please contact the responsible support.
