<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>UltraCalc Layout Editor</title>
    <style>
        :root {
            --bg-app: #eceef1;
            --bg-panel: #ffffff;
            --bg-subtle: #f5f6f8;
            --bg-input: #ffffff;
            --border: #dcdfe5;
            --border-strong: #c2c7d0;
            --text-main: #181a1f;
            --text-muted: #5e6573;
            --accent: #0969da;
            --accent-subtle: rgba(9, 105, 218, 0.1);
            --ink-btn: #181a1f;
            --ink-btn-hover: #2c303a;
            --danger: #c62828;
            --font-sans: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            --font-mono: ui-monospace, "SF Mono", Menlo, Consolas, monospace;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: var(--font-sans);
            background: var(--bg-app);
            color: var(--text-main);
            height: 100vh;
            height: 100dvh;
            display: flex;
            flex-direction: column;
            overflow: hidden;
            font-size: 13px;
            line-height: 1.4;
        }

        .topbar {
            height: 46px;
            background: var(--bg-panel);
            border-bottom: 1px solid var(--border);
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 0 14px;
            flex-shrink: 0;
            gap: 12px;
        }

        .topbar-left {
            display: flex;
            align-items: center;
            gap: 14px;
        }

        .brand {
            font-weight: 700;
            font-size: 14px;
            color: var(--text-main);
            white-space: nowrap;
        }

        .mode-tabs {
            display: inline-flex;
            background: var(--bg-subtle);
            border: 1px solid var(--border-strong);
            border-radius: 4px;
            padding: 2px;
            gap: 2px;
        }

        .mode-tabs button {
            border: none;
            background: transparent;
            color: var(--text-muted);
            padding: 3px 10px;
            border-radius: 3px;
            font-size: 12px;
            font-weight: 500;
        }

        .mode-tabs button:hover {
            background: rgba(0, 0, 0, 0.04);
            color: var(--text-main);
        }

        .mode-tabs button.active {
            background: var(--bg-panel);
            color: var(--text-main);
            box-shadow: 0 1px 2px rgba(0, 0, 0, 0.08);
        }

        .topbar-actions {
            display: flex;
            align-items: center;
            gap: 6px;
        }

        button {
            font-family: inherit;
            cursor: pointer;
            border: 1px solid var(--border-strong);
            background: var(--bg-panel);
            color: var(--text-main);
            padding: 5px 11px;
            border-radius: 4px;
            font-size: 12px;
            font-weight: 500;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            touch-action: manipulation;
            -webkit-tap-highlight-color: transparent;
        }

        button:hover {
            background: var(--bg-subtle);
            border-color: #9aa1ad;
        }

        button:active {
            background: #e8eaee;
        }

        button.success {
            background: var(--ink-btn);
            border-color: var(--ink-btn);
            color: #ffffff;
            font-weight: 600;
        }

        button.success:hover {
            background: var(--ink-btn-hover);
            border-color: var(--ink-btn-hover);
        }

        button.danger {
            background: var(--bg-panel);
            border-color: #e5b4b4;
            color: var(--danger);
        }

        button.danger:hover {
            background: #fef2f2;
            border-color: var(--danger);
        }

        /* Main Workspace */
        .workspace {
            display: flex;
            flex: 1;
            min-height: 0;
        }

        .sidebar-left {
            width: 258px;
            background: var(--bg-panel);
            border-right: 1px solid var(--border);
            padding: 14px;
            overflow-y: auto;
            display: flex;
            flex-direction: column;
            gap: 14px;
            flex-shrink: 0;
        }

        .stage {
            flex: 1;
            background: var(--bg-app);
            display: flex;
            flex-direction: column;
            overflow: auto;
            padding: 28px;
            position: relative;
        }

        .stage-viewport {
            margin: auto;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        #canvas-stage-frame {
            position: relative;
            flex-shrink: 0;
        }

        #canvas-wrapper {
            position: relative;
            box-shadow: 0 4px 18px rgba(0, 0, 0, 0.12), 0 0 0 1px rgba(0, 0, 0, 0.14);
            border-radius: 10px;
            overflow: hidden;
            user-select: none;
            -webkit-user-select: none;
            touch-action: none;
        }

        #calc-canvas {
            position: relative;
            width: 100%;
            height: 100%;
            touch-action: none;
        }

        #calc-canvas.show-grid {
            background-image:
                linear-gradient(to right, rgba(128, 128, 128, 0.12) 1px, transparent 1px),
                linear-gradient(to bottom, rgba(128, 128, 128, 0.12) 1px, transparent 1px);
            background-size: 8px 8px;
        }

        .canvas-resize-handle {
            position: absolute;
            right: 0;
            bottom: 0;
            width: 18px;
            height: 18px;
            cursor: nwse-resize;
            touch-action: none;
            z-index: 50;
            background: linear-gradient(135deg,
                transparent 46%,
                rgba(100, 108, 120, 0.55) 46%,
                rgba(100, 108, 120, 0.55) 56%,
                transparent 56%,
                transparent 70%,
                rgba(100, 108, 120, 0.55) 70%,
                rgba(100, 108, 120, 0.55) 80%,
                transparent 80%);
        }

        .canvas-dim-badge {
            position: absolute;
            bottom: 10px;
            left: 12px;
            font-family: var(--font-mono);
            font-variant-numeric: tabular-nums;
            font-size: 11px;
            color: var(--text-muted);
            background: var(--bg-panel);
            border: 1px solid var(--border);
            padding: 3px 8px;
            border-radius: 4px;
            pointer-events: none;
        }

        /* Draggable Elements on Canvas */
        .ui-element {
            position: absolute;
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: move;
            user-select: none;
            -webkit-user-select: none;
            -webkit-tap-highlight-color: transparent;
            touch-action: none;
        }

        .ui-element.preview-mode {
            cursor: pointer;
            touch-action: manipulation;
        }

        .ui-element.preview-mode:hover {
            filter: brightness(0.96);
        }

        .ui-element.preview-mode:active {
            filter: brightness(0.9);
            transform: translateY(0.5px);
        }

        .ui-element.selected {
            outline: 2px solid var(--accent);
            outline-offset: 1px;
            z-index: 20;
        }

        .ui-element .el-resize {
            position: absolute;
            right: -5px;
            bottom: -5px;
            width: 10px;
            height: 10px;
            background: #ffffff;
            border: 1.5px solid var(--accent);
            border-radius: 2px;
            cursor: nwse-resize;
            touch-action: none;
            display: none;
        }

        .ui-element.selected .el-resize {
            display: block;
        }

        .ui-display {
            justify-content: flex-end;
            padding: 0 14px;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
            font-variant-numeric: tabular-nums;
        }

        /* Right Sidebar: Inspector */
        .sidebar-right {
            width: 276px;
            background: var(--bg-panel);
            border-left: 1px solid var(--border);
            padding: 14px;
            overflow-y: auto;
            display: flex;
            flex-direction: column;
            gap: 14px;
            flex-shrink: 0;
        }

        .section {
            display: flex;
            flex-direction: column;
            gap: 9px;
            padding-bottom: 14px;
            border-bottom: 1px solid var(--border);
        }

        .section:last-child {
            border-bottom: none;
            padding-bottom: 0;
        }

        .section-header-row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            gap: 8px;
        }

        .section-title {
            font-size: 12px;
            font-weight: 600;
            color: var(--text-main);
        }

        .field {
            display: flex;
            flex-direction: column;
            gap: 4px;
        }

        .field-row {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 8px;
        }

        label {
            font-size: 11px;
            font-weight: 500;
            color: var(--text-muted);
        }

        input[type="text"],
        input[type="number"],
        select {
            width: 100%;
            height: 30px;
            background: var(--bg-input);
            border: 1px solid var(--border-strong);
            color: var(--text-main);
            padding: 0 8px;
            border-radius: 4px;
            font-size: 12px;
            font-family: inherit;
        }

        input[type="number"] {
            font-family: var(--font-mono);
            font-variant-numeric: tabular-nums;
        }

        .color-field {
            display: flex;
            align-items: center;
            gap: 6px;
            background: var(--bg-input);
            border: 1px solid var(--border-strong);
            border-radius: 4px;
            padding: 3px 7px 3px 3px;
            height: 30px;
        }

        .color-field input[type="color"] {
            width: 24px;
            height: 22px;
            border: 1px solid var(--border);
            border-radius: 3px;
            padding: 0;
            background: transparent;
            cursor: pointer;
            flex-shrink: 0;
        }

        .color-hex {
            font-family: var(--font-mono);
            font-size: 11px;
            color: var(--text-main);
            user-select: all;
        }

        input:focus, select:focus {
            outline: none;
            border-color: var(--accent);
            box-shadow: 0 0 0 2px var(--accent-subtle);
        }

        .preset-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 6px;
        }

        .preset-grid button {
            font-size: 11px;
            padding: 4px;
        }

        .ttf-dropzone {
            border: 1px dashed var(--border-strong);
            border-radius: 4px;
            padding: 11px 10px;
            text-align: center;
            font-size: 12px;
            color: var(--text-muted);
            cursor: pointer;
            background: var(--bg-subtle);
        }

        .ttf-dropzone:hover, .ttf-dropzone.dragover {
            border-color: var(--accent);
            color: var(--text-main);
            background: var(--bg-panel);
        }

        .ttf-status {
            font-family: var(--font-mono);
            font-size: 11px;
            color: #156d3c;
            word-break: break-all;
        }

        .checkbox-row {
            display: flex;
            align-items: center;
            gap: 7px;
            font-size: 12px;
            color: var(--text-main);
            cursor: pointer;
        }

        .checkbox-row input[type="checkbox"] {
            accent-color: var(--accent);
            cursor: pointer;
        }

        .hint {
            font-size: 11px;
            color: var(--text-muted);
            line-height: 1.45;
        }

        .hint code {
            font-family: var(--font-mono);
            font-size: 10.5px;
            background: var(--bg-subtle);
            border: 1px solid var(--border);
            padding: 0 4px;
            border-radius: 3px;
            color: var(--text-main);
        }

        .modal-backdrop {
            position: fixed;
            inset: 0;
            background: rgba(15, 18, 24, 0.45);
            display: none;
            align-items: center;
            justify-content: center;
            z-index: 100;
        }

        .modal-backdrop.open {
            display: flex;
        }

        .modal {
            background: var(--bg-panel);
            border: 1px solid var(--border-strong);
            border-radius: 6px;
            width: 640px;
            max-width: 92vw;
            max-height: 82vh;
            display: flex;
            flex-direction: column;
            overflow: hidden;
            box-shadow: 0 12px 32px rgba(0, 0, 0, 0.18);
        }

        .modal-header {
            padding: 10px 14px;
            border-bottom: 1px solid var(--border);
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: var(--bg-subtle);
        }

        .modal-header-actions {
            display: flex;
            align-items: center;
            gap: 6px;
        }

        .modal-body {
            padding: 14px;
            overflow: auto;
        }

        pre {
            background: var(--bg-subtle);
            border: 1px solid var(--border);
            border-radius: 4px;
            padding: 12px;
            font-family: var(--font-mono);
            font-size: 12px;
            line-height: 1.45;
            overflow-x: auto;
            color: var(--text-main);
        }

        .mobile-nav {
            display: none;
        }

        .mobile-selection-bar {
            display: none;
        }

        #btn-mobile-back {
            display: none;
        }

        @media (max-width: 820px) {
            .topbar {
                height: auto;
                flex-direction: column;
                align-items: stretch;
                padding: 8px 10px;
                gap: 6px;
            }

            .topbar-left {
                width: 100%;
                justify-content: space-between;
                gap: 8px;
            }

            .topbar-actions {
                width: 100%;
                display: grid;
                grid-template-columns: repeat(4, 1fr);
                gap: 6px;
            }

            .topbar button {
                padding: 6px 6px;
                font-size: 11.5px;
                min-height: 34px;
                white-space: nowrap;
            }

            .workspace {
                flex-direction: column;
            }

            .sidebar-left,
            .sidebar-right,
            .stage {
                display: none;
                width: 100%;
                flex: 1;
                min-height: 0;
                border-right: none;
                border-left: none;
            }

            .sidebar-left.mobile-active,
            .sidebar-right.mobile-active {
                display: flex;
                padding-bottom: 24px;
            }

            .stage.mobile-active {
                display: flex;
                flex-direction: column;
                justify-content: space-between;
                align-items: center;
                padding: 12px 10px 8px;
                gap: 8px;
                overflow: hidden;
            }

            .stage-viewport {
                flex: 1;
                width: 100%;
                min-height: 0;
                display: flex;
                flex-direction: column;
                align-items: center;
                justify-content: center;
                gap: 6px;
            }

            .canvas-dim-badge {
                position: static;
                align-self: center;
                font-size: 11px;
                padding: 2px 8px;
            }

            .mobile-selection-bar {
                display: flex;
                width: 100%;
                height: 44px;
                flex-shrink: 0;
                background: var(--bg-panel);
                border: 1px solid var(--border);
                border-radius: 6px;
                padding: 0 8px 0 12px;
                align-items: center;
                justify-content: space-between;
                gap: 8px;
            }

            .mobile-sel-label {
                font-size: 12px;
                color: var(--text-muted);
                overflow: hidden;
                text-overflow: ellipsis;
                white-space: nowrap;
            }

            .mobile-sel-label strong {
                color: var(--text-main);
            }

            .mobile-sel-actions {
                display: flex;
                align-items: center;
                gap: 6px;
                flex-shrink: 0;
            }

            .mobile-sel-actions button {
                min-height: 30px;
                padding: 4px 10px;
                font-size: 12px;
            }

            #btn-mobile-back {
                display: inline-flex;
                min-height: 30px;
                padding: 4px 10px;
                font-size: 12px;
            }

            input[type="text"],
            input[type="number"],
            select {
                min-height: 38px;
                font-size: 16px;
            }

            .color-field {
                height: 38px;
            }

            .color-field input[type="color"] {
                width: 30px;
                height: 28px;
            }

            .ui-element .el-resize {
                width: 22px;
                height: 22px;
                right: -9px;
                bottom: -9px;
                border-radius: 50%;
                border-width: 2px;
                box-shadow: 0 1px 3px rgba(0, 0, 0, 0.25);
            }

            .canvas-resize-handle {
                width: 24px;
                height: 24px;
            }

            .mobile-nav {
                height: 50px;
                background: var(--bg-panel);
                border-top: 1px solid var(--border);
                display: grid;
                grid-template-columns: repeat(3, 1fr);
                flex-shrink: 0;
            }

            .mobile-nav button {
                border: none;
                border-radius: 0;
                background: transparent;
                color: var(--text-muted);
                font-size: 12px;
                font-weight: 600;
                height: 100%;
            }

            .mobile-nav button.active {
                background: var(--bg-subtle);
                color: var(--accent);
                border-top: 2px solid var(--accent);
            }
        }
    </style>
</head>
<body>
    <header class="topbar">
        <div class="topbar-left">
            <div class="brand">UltraCalc</div>
            <div class="mode-tabs">
                <button id="btn-mode-design" class="active" onclick="setMode('design')">Design</button>
                <button id="btn-mode-preview" onclick="set