import argparse
import urllib.request
import subprocess
from urllib.parse import urlparse
from pathlib import Path

parser = argparse.ArgumentParser(description='Connext DDS Installation Helper')
parser.add_argument("-V", "--version", help="Connext DDS version to install", required=True)
parser.add_argument("-H", "--host-url", help="URL to download Connext DDS Host installer", required=True)
parser.add_argument("-T", "--target-url", help="URL to download Connext DDS Target installer", nargs='+')
parser.add_argument("-O", "--output-dir", help="Output directory where to save installers, and install Connext", type=Path, default=Path.cwd() / "connextdds")

args = parser.parse_args()

output_dir = args.output_dir
output_dir.mkdir(parents=True, exist_ok=True)

def download_file(url: str, output_dir: Path) -> Path:
    parsed_url = urlparse(url)
    filename = Path(parsed_url.path).name
    output_path = output_dir / filename

    print(f"Downloading {url} to {output_path}...")
    urllib.request.urlretrieve(url, output_path)
    print(f"Downloaded {filename}.")

    return output_path

host_installer = None
target_installers = []

try:
    host_installer = download_file(args.host_url, output_dir)
    target_installers = [download_file(url, output_dir) for url in args.target_url]

    connextdds_dir = output_dir / f"rti_connext_dds-{args.version}"
    subprocess.run([str(host_installer), "--mode", "unattended", "--unattendedmodeui", "minimalWithDialogs", "--prefix", str(output_dir)], check=True)
    rtipkginstall = connextdds_dir / "bin" / "rtipkginstall.bat"
    if not rtipkginstall.exists():
        raise RuntimeError(f"rtipkginstall not found: {rtipkginstall}")
    for target_installer in target_installers:
        subprocess.run([str(rtipkginstall), "-u", str(target_installer)], check=True)

    print(f"Connext DDS installed in {connextdds_dir}")
finally:
    if host_installer and host_installer.exists():
        host_installer.unlink()
    for target_installer in target_installers:
        if target_installer.exists():
            target_installer.unlink()
