0404 謝睿哲
*todo*
1. head 和 body的模型要處理一下
2. 我在imgui新增的textbox不能用 只有+/-符號可以用 我想是因為鍵盤輸入被MainScene搶走了 看要修還是要換
3. 我把reset model pos放在mainscene::UpdateModel裡面 看有沒有需要換地方

*modify*
1. MainScene
       1. +enum Body,Action
       2. +float alphas[PARTSNUM], betas[PARTSNUM], gammas[PARTSNUM]紀錄每個身體部位的角度
        3. +isActionChange 切換躺下/站立 
    1. +MainScene::SetRotate() 記錄在imgui改變的角度
    2. +MainScene::bodyRotateMatrix()簡化updateModel
    3. LoadModel 把model 改成10個部分
2. ControlWindow
    1. +新增editor視窗

3. 我把reset model pos放在mainscene::UpdateModel裡面

code蠻髒的
隨意改