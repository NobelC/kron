# Release Signing and Verification

As a security-focused tool, `kls` ensures the cryptographic integrity and authenticity of its release packages. Every official release includes detached GPG signatures (`.asc` files) generated during the CI/CD build.

---

## 🔒 Verification for Users

To verify that a downloaded package (e.g., `.deb`, `.rpm`, or `.tar.gz`) is authentic and has not been tampered with:

### 1. Obtain the Public Key
Import the official `kls` release public key. It can be found in the repository at `docs/kls-release.pub` or retrieved from public key servers (once registered):

```bash
gpg --import docs/kls-release.pub
# Or from a keyserver (example key ID):
# gpg --keyservers hkps://keys.openpgp.org --recv-keys <KEY_ID>
```

### 2. Download the Signature
Download both the release asset and its corresponding signature file (ending in `.asc`) from the GitHub Releases page. They must be placed in the same directory.

### 3. Verify the Integrity
Run the following GPG command on the target package:

```bash
gpg --verify kls-0.1.1-Linux-x86_64.deb.asc kls-0.1.1-Linux-x86_64.deb
```

If successful, you will see an output similar to:
```text
gpg: Signature made Mon May 18 20:30:00 2026 UTC
gpg:                using RSA key <KEY_ID>
gpg: Good signature from "Jeremy Nobel (kls Release Key) <jeremynobel16@gmail.com>" [ultimate]
```

---

## 🔑 Setup Guide for Maintainers (CI/CD Automations)

The release workflow is designed to sign all packaging formats (`.deb`, `.rpm`, `.tar.gz`) dynamically at tag time. Follow these steps to enable automatic signing:

### 1. Generate a Dedicated GPG Key Pair
Generate a robust, password-protected (or passwordless, restricted) RSA/Ed25519 key pair dedicated to signing releases:

```bash
gpg --full-generate-key
```
*Recommendations:*
*   **Key Type**: RSA and RSA (default) or ECC
*   **Key Size**: 4096 bits
*   **Expiration**: 2–3 years (or no expiration, depending on rotate policies)
*   **Real Name**: `Jeremy Nobel (kls Release Key)`
*   **Email Address**: `jeremynobel16@gmail.com`

### 2. Export the Keys
Export the public key to include it in the repository:
```bash
gpg --armor --export "Jeremy Nobel (kls Release Key)" > docs/kls-release.pub
```

Export the private key as an ASCII-armored block (to be configured as a repository secret):
```bash
gpg --armor --export-secret-keys "Jeremy Nobel (kls Release Key)" > kls-private.key
```

> [!WARNING]
> Keep `kls-private.key` strictly confidential. Delete the local copy once configured in the GitHub repository.

### 3. Add GitHub Repository Secrets
Go to your GitHub repository: **Settings > Secrets and variables > Actions** and create the following secrets:

1.  **`GPG_PRIVATE_KEY`**: Paste the entire contents of the exported private key file (`kls-private.key` including the headers `-----BEGIN PGP PRIVATE KEY BLOCK-----` and `-----END PGP PRIVATE KEY BLOCK-----`).
2.  **`GPG_PASSPHRASE`**: (Optional) If you protected your GPG key with a passphrase, store it here. If the key has no passphrase, this secret can be omitted.

### 4. How the Workflow Operates
The `.github/workflows/release.yml` includes a conditional step that imports your key and signs all build artifacts:
```yaml
      - name: Import GPG key and Sign Assets
        if: ${{ secrets.GPG_PRIVATE_KEY != '' }}
        ...
```
If you haven't added the secrets yet, the signing process is skipped gracefully without interrupting regular non-signing release pipelines.
