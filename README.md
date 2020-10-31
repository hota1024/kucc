# ⬛ kucc

## 🔧 Development

```bash
# 🔽 Clone this repository.
gh clone hota1024/kucc
cd kucc

# 🐳 Build image.
docker build -t kucc .

# ✅ Run test.
docker run --rm -v $(pwd):/kucc -w /kucc kucc make
```
