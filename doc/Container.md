# Container deployment

The Dashboard-OPCUA-Client repository automatically builds container images for linux/amd64 platforms.

This container images can be used in containerized deployment scenarios to execute the gateway.
The `configuration.json` needs to be mounted with a volume mount into the container.

## Deployment

1. Prepare the `configuration.json` as per [Configuration](./Configuration.md)
2. Pull the container image:

   ``` shell
   docker pull ghcr.io/umati/dashboard-opcua-client
   ```

3. Run the image with mounting the configuration into the container:

    ``` shell
    docker run -it --rm -v /path/to//configuration.json:/app/configuration.json --name=dashboard-opcua-client ghcr.io/umati/dashboard-opcua-client:v*.*.*
    ```

    Replace PATH to `configuration.json` and container version to the desired version as per [Releases](https://github.com/umati/Dashboard-OPCUA-Client/releases).
